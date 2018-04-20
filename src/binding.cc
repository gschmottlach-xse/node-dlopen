#include "uv.h"
#include "node.h"
#include "node_buffer.h"
#include "node_version.h"
#include "nan.h"

namespace {

/**
 * dlopen()
 */

NAN_METHOD(Dlopen) {
  Nan::EscapableHandleScope();

  const char *filename;
  if (info[0]->IsNull()) {
    filename = NULL;
  } else if (info[0]->IsString()) {
    v8::String::Utf8Value name(info[0]);
    filename = *name;
  } else {
    return Nan::ThrowTypeError("a string filename, or null must be passed as the first argument");
  }

  v8::Local<v8::Object> buf = info[1].As<v8::Object>();

  uv_lib_t *lib = reinterpret_cast<uv_lib_t *>(node::Buffer::Data(buf));
  int r = 0;

#if NODE_VERSION_AT_LEAST(0, 7, 9)
  r = uv_dlopen(filename, lib);
#else
  uv_err_t err = uv_dlopen(filename, lib);
  if (err.code != UV_OK) {
    r = err.code;
  }
#endif

  info.GetReturnValue().Set(Nan::New<v8::Integer>(r));
}

/**
 * dlclose()
 */

NAN_METHOD(Dlclose) {
  Nan::EscapableHandleScope();

  v8::Local<v8::Object> buf = info[0].As<v8::Object>();

  uv_lib_t *lib = reinterpret_cast<uv_lib_t *>(node::Buffer::Data(buf));

  uv_dlclose(lib);

  info.GetReturnValue().SetUndefined();
}

/**
 * dlsym()
 */

NAN_METHOD(Dlsym) {
  Nan::EscapableHandleScope();

  v8::Local<v8::Object> buf = info[0].As<v8::Object>();
  v8::String::Utf8Value name(info[1]);
  v8::Local<v8::Object> sym_buf = info[2].As<v8::Object>();

  uv_lib_t *lib = reinterpret_cast<uv_lib_t *>(node::Buffer::Data(buf));
  void *sym = reinterpret_cast<void *>(node::Buffer::Data(sym_buf));

  int r = 0;
#if NODE_VERSION_AT_LEAST(0, 7, 9)
  r = uv_dlsym(lib, *name, &sym);
#else
  uv_err_t err = uv_dlsym(lib, *name, &sym);
  if (err.code != UV_OK) {
    r = err.code;
  }
#endif

  info.GetReturnValue().Set(Nan::New<v8::Integer>(r));
}

/**
 * dlerror()
 */

#if NODE_VERSION_AT_LEAST(0, 7, 9)

NAN_METHOD(Dlerror) {
  Nan::EscapableHandleScope();

  v8::Local<v8::Object> buf = info[0].As<v8::Object>();

  uv_lib_t *lib = reinterpret_cast<uv_lib_t *>(node::Buffer::Data(buf));

  info.GetReturnValue().Set(Nan::New<v8::String>(uv_dlerror(lib)).ToLocalChecked());
}

#endif

} // anonymous namespace

void init (v8::Handle<v8::Object> target) {
  Nan::HandleScope();

  target->Set(Nan::New<v8::String>("sizeof_uv_lib_t").ToLocalChecked(), Nan::New<v8::Uint32>(static_cast<uint32_t>(sizeof(uv_lib_t))));
  target->Set(Nan::New<v8::String>("sizeof_void_ptr").ToLocalChecked(), Nan::New<v8::Uint32>(static_cast<uint32_t>(sizeof(void *))));

  Nan::SetMethod(target, "dlopen", Dlopen);
  Nan::SetMethod(target, "dlclose", Dlclose);
  Nan::SetMethod(target, "dlsym", Dlsym);
#if NODE_VERSION_AT_LEAST(0, 7, 9)
  // libuv with node < 0.7.9 didn't have any dlerror() function
  Nan::SetMethod(target, "dlerror", Dlerror);
#endif
}
NODE_MODULE(binding, init);
