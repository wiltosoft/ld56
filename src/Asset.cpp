/*
 * Asset.cpp
 *
 * Created by miles
*/

#include "Asset.h"

std::list<Asset> Asset::xAssets = {};

Asset::Asset(const char *file)
    : uBytesTotal(0)
    , uBytesDownloaded(0)
{

#ifdef __EMSCRIPTEN__
    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.onprogress = Asset::OnProgress;
    attr.onsuccess = Asset::OnSuccess;
    attr.onerror = Asset::OnError;
    attr.userData = this;
    emscripten_fetch(&attr, file);
    eState = DOWNLOADING;
#else
    auto h = fopen(file, "rb");
    if(h == NULL){
        eState = FAILED;
        _ERROR("Failed to open file '%s'", file);
        return;
    }

    fseek(h, 0L, SEEK_END);
    uBytesTotal = ftell(h);
    pBuffer = malloc(uBytesTotal + 1);
    rewind(h);
    uBytesDownloaded = fread(pBuffer, 1, uBytesTotal, h);

    fclose(h);

    ((char*)pBuffer)[uBytesTotal] = 0;

    _f.ptr = 0;
    _f.size = uBytesTotal;
    _f.buf = pBuffer;

    eState = COMPLETE;
#endif
}




#ifdef __EMSCRIPTEN__
void Asset::OnProgress(emscripten_fetch_t *fetch)
{
    auto xAsset = (Asset*)fetch->userData;
    xAsset->uBytesTotal = fetch->totalBytes;
    xAsset->uBytesDownloaded = fetch->dataOffset;

    _INFO("Download [%s] %.2f%%, %llu of %llu", fetch->url, fetch->dataOffset * 100.0 / fetch->totalBytes, fetch->dataOffset, fetch->totalBytes);
}


void Asset::OnSuccess(emscripten_fetch_t *fetch)
{
    auto xAsset = (Asset*)fetch->userData;

    xAsset->uBytesTotal = fetch->totalBytes;
    xAsset->uBytesDownloaded = fetch->dataOffset;
    xAsset->pBuffer = malloc(fetch->totalBytes + 1);
    memcpy(xAsset->pBuffer, fetch->data, fetch->totalBytes);
    ((char*)xAsset->pBuffer)[xAsset->uBytesTotal] = 0;

    _INFO("Download [%s] complete", fetch->url);

    xAsset->_f.ptr = 0;
    xAsset->_f.size = xAsset->uBytesTotal;
    xAsset->_f.buf = xAsset->pBuffer;

    xAsset->eState = COMPLETE;

    emscripten_fetch_close(fetch);
}


void Asset::OnError(emscripten_fetch_t *fetch)
{
    auto xAsset = (Asset*)fetch->userData;
    xAsset->uBytesDownloaded = 0;
    _ERROR("Download [%s] failed, %s", fetch->url, fetch->statusText);
    xAsset->eState = FAILED;

    emscripten_fetch_close(fetch);
}
#endif