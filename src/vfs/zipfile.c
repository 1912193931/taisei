
#include "zipfile.h"
#include "zippath.h"

static VFSZipFileTLS* vfs_zipfile_get_tls(VFSNode *node);

static zip_int64_t vfs_zipfile_srcfunc(void *userdata, void *data, zip_uint64_t len, zip_source_cmd_t cmd) {
    VFSNode *zipnode = userdata;
    VFSZipFileData *zdata = zipnode->data;
    VFSZipFileTLS *tls = vfs_zipfile_get_tls(zipnode);
    VFSNode *source = zdata->source;

    switch(cmd) {
        case ZIP_SOURCE_OPEN: {
            if(!source->funcs->open) {
                return -1;
            }

            tls->stream = source->funcs->open(source, VFS_MODE_READ);

            if(!tls->stream) {
                return -1;
            }

            return 0;
        }

        case ZIP_SOURCE_CLOSE: {
            if(tls->stream) {
                SDL_RWclose(tls->stream);
                tls->stream = NULL;
            }
            return 0;
        }

        case ZIP_SOURCE_STAT: {
            zip_stat_t *stat = data;
            zip_stat_init(stat);

            stat->valid = ZIP_STAT_COMP_METHOD | ZIP_STAT_ENCRYPTION_METHOD;
            stat->comp_method = ZIP_CM_STORE;
            stat->encryption_method = ZIP_EM_NONE;

            if(tls->stream) {
                stat->valid |= ZIP_STAT_SIZE | ZIP_STAT_COMP_SIZE;
                stat->size = SDL_RWsize(tls->stream);
                stat->comp_size = stat->size;
            }

            return sizeof(struct zip_stat);
        }

        case ZIP_SOURCE_READ: {
            zip_int64_t r;
            r = SDL_RWread(tls->stream, data, 1, len);
            return r;
        }

        case ZIP_SOURCE_SEEK: {
            struct zip_source_args_seek *s;
            s = ZIP_SOURCE_GET_ARGS(struct zip_source_args_seek, data, len, &tls->error);
            return SDL_RWseek(tls->stream, s->offset, s->whence);
        }

        case ZIP_SOURCE_TELL: {
            return SDL_RWtell(tls->stream);
        }

        case ZIP_SOURCE_ERROR: {
            return zip_error_to_data(&tls->error, data, len);
        }

        case ZIP_SOURCE_SUPPORTS: {
            return ZIP_SOURCE_SUPPORTS_SEEKABLE;
        }

        case ZIP_SOURCE_FREE: {
            return 0;
        }

        default: {
            return -1;
        }
    }
}

void vfs_zipfile_free_tls(VFSZipFileTLS *tls) {
    if(tls->zip) {
        zip_discard(tls->zip);
    }

    if(tls->stream) {
        SDL_RWclose(tls->stream);
    }

    free(tls);
}

static void vfs_zipfile_free(VFSNode *node) {
    if(node) {
        VFSZipFileData *zdata = node->data;

        if(zdata) {
            SDL_TLSID tls_id = ((VFSZipFileData*)node->data)->tls_id;
            VFSZipFileTLS *tls = SDL_TLSGet(tls_id);

            if(tls) {
                vfs_zipfile_free_tls(tls);
                SDL_TLSSet(tls_id, NULL, NULL);
            }

            vfs_decref(zdata->source);
            hashtable_free(zdata->pathmap);
            free(zdata);
        }
    }
}

static VFSInfo vfs_zipfile_query(VFSNode *node) {
    return (VFSInfo) {
        .exists = true,
        .is_dir = true,
    };
}

static char* vfs_zipfile_syspath(VFSNode *node) {
    VFSZipFileData *zdata = node->data;

    if(zdata->source->funcs->syspath) {
        return zdata->source->funcs->syspath(zdata->source);
    }

    return NULL;
}

static char* vfs_zipfile_repr(VFSNode *node) {
    VFSZipFileData *zdata = node->data;
    char *srcrepr = vfs_repr_node(zdata->source, false);
    char *ziprepr = strfmt("zip archive %s", srcrepr);
    free(srcrepr);
    return ziprepr;
}

static VFSNode* vfs_zipfile_locate(VFSNode *node, const char *path) {
    VFSZipFileTLS *tls = vfs_zipfile_get_tls(node);
    VFSZipFileData *zdata = node->data;
    zip_int64_t idx = (zip_int64_t)((uintptr_t)hashtable_get_string(zdata->pathmap, path) - 1);

    if(idx < 0) {
        idx = zip_name_locate(tls->zip, path, 0);
    }

    if(idx < 0) {
        return NULL;
    }

    VFSNode *n = vfs_alloc(true);
    vfs_zippath_init(n, node, tls, idx);
    return n;
}

const char* vfs_zipfile_iter_shared(VFSNode *node, VFSZipFileData *zdata, VFSZipFileIterData *idata, VFSZipFileTLS *tls) {
    const char *r = NULL;

    for(; !r && idata->idx < idata->num; ++idata->idx) {
        const char *p = zip_get_name(tls->zip, idata->idx, 0);

        if(idata->prefix) {
            if(strstartswith(p, idata->prefix)) {
                p += idata->prefix_len;
            } else {
                continue;
            }
        }

        if(!*p) {
            continue;
        }

        char *sep = strchr(p, '/');

        if(sep) {
            if(*(sep + 1)) {
                // path is inside a subdirectory, we want only top-level entries
                continue;
            }

            // separator is the last character in the string
            // this is a top-level subdirectory
            // strip the trailing slash

            free(idata->allocated);
            idata->allocated = strdup(p);
            *strchr(idata->allocated, '/') = 0;
            r = idata->allocated;
        } else {
            r = p;
        }
    }

    return r;
}

static const char* vfs_zipfile_iter(VFSNode *node, void **opaque) {
    VFSZipFileData *zdata = node->data;
    VFSZipFileIterData *idata = *opaque;
    VFSZipFileTLS *tls = vfs_zipfile_get_tls(node);

    if(!idata) {
        *opaque = idata = calloc(1, sizeof(VFSZipFileIterData));
        idata->num = zip_get_num_entries(tls->zip, 0);
    }

    return vfs_zipfile_iter_shared(node, zdata, idata, tls);
}

void vfs_zipfile_iter_stop(VFSNode *node, void **opaque) {
    VFSZipFileIterData *idata = *opaque;

    if(idata) {
        free(idata->allocated);
        free(idata);
        *opaque = NULL;
    }
}

static VFSNodeFuncs vfs_funcs_zipfile = {
    .repr = vfs_zipfile_repr,
    .query = vfs_zipfile_query,
    .free = vfs_zipfile_free,
    .syspath = vfs_zipfile_syspath,
    //.mount = vfs_zipfile_mount,
    .locate = vfs_zipfile_locate,
    .iter = vfs_zipfile_iter,
    .iter_stop = vfs_zipfile_iter_stop,
    //.mkdir = vfs_zipfile_mkdir,
    //.open = vfs_zipfile_open,
};

static void vfs_zipfile_init_pathmap(VFSNode *node) {
    VFSZipFileData *zdata = node->data;
    VFSZipFileTLS *tls = vfs_zipfile_get_tls(node);
    zdata->pathmap = hashtable_new_stringkeys(HT_DYNAMIC_SIZE);
    zip_int64_t num = zip_get_num_entries(tls->zip, 0);

    for(zip_int64_t i = 0; i < num; ++i) {
        const char *original = zip_get_name(tls->zip, i, 0);
        char normalized[strlen(original) + 1];

        vfs_path_normalize(original, normalized);

        char *c = strchr(normalized, 0) - 1;
        if(*c == '/') {
            *c = 0;
        }

        if(strcmp(original, normalized)) {
            hashtable_set_string(zdata->pathmap, normalized, (void*)((uintptr_t)i + 1));
        }
    }
}

static VFSZipFileTLS* vfs_zipfile_get_tls(VFSNode *node) {
    VFSZipFileData *zdata = node->data;
    VFSZipFileTLS *tls = SDL_TLSGet(zdata->tls_id);

    if(tls) {
        return tls;
    }

    tls = calloc(1, sizeof(VFSZipFileTLS));
    SDL_TLSSet(zdata->tls_id, tls, (void(*)(void*))vfs_zipfile_free_tls);

    zip_source_t *src = zip_source_function_create(vfs_zipfile_srcfunc, node, &tls->error);
    zip_t *zip = tls->zip = zip_open_from_source(src, ZIP_RDONLY, &tls->error);

    if(!zip) {
        char *r = vfs_repr_node(zdata->source, true);
        vfs_set_error("Failed to open zip archive '%s': %s", r, zip_error_strerror(&tls->error));
        free(r);
        free(tls);
        zip_source_free(src);
        return NULL;
    }

    return tls;
}

bool vfs_zipfile_init(VFSNode *node, VFSNode *source) {
    VFSZipFileData *zdata = calloc(1, sizeof(VFSZipFileData));
    zdata->source = source;
    zdata->tls_id = SDL_TLSCreate();

    node->data = zdata;
    node->type = VNODE_ZIPFILE;
    node->funcs = &vfs_funcs_zipfile;

    vfs_zipfile_init_pathmap(node);
    return true;
}

#include "syspath.h"

static void vfs_zipfile_test_zip(VFSNode *zipnode) {
    vfs_mount(vfs_root, "zip", zipnode);
}

void vfs_zipfile_test(void) {
    VFSNode *srcnode = vfs_alloc(false);
    VFSNode *zipnode = vfs_alloc(false);

    log_debug("syspath");
    if(!vfs_syspath_init(srcnode, "/tmp/ts/test.zip")) {
        log_fatal("wtf");
    }
    log_debug("ok");

    log_debug("zipfile");
    if(!vfs_zipfile_init(zipnode, srcnode)) {
        log_fatal("wtf 2");
    }
    log_debug("ok");

    vfs_zipfile_test_zip(zipnode);
}
