#include <nan.h>

void XxHash32(const Nan::FunctionCallbackInfo<v8::Value>& info);
void XxHash64(const Nan::FunctionCallbackInfo<v8::Value>& info);
void XxHash3(const Nan::FunctionCallbackInfo<v8::Value>& info);
void XxHash3_128(const Nan::FunctionCallbackInfo<v8::Value>& info);

void XxHash32File(const Nan::FunctionCallbackInfo<v8::Value>& info);
void XxHash64File(const Nan::FunctionCallbackInfo<v8::Value>& info);
void XxHash3File(const Nan::FunctionCallbackInfo<v8::Value>& info);
void XxHash3_128_File(const Nan::FunctionCallbackInfo<v8::Value>& info);

void CreateXxHash32State(const Nan::FunctionCallbackInfo<v8::Value>& info);
void CreateXxHash64State(const Nan::FunctionCallbackInfo<v8::Value>& info);
void CreateXxHash3State(const Nan::FunctionCallbackInfo<v8::Value>& info);
void CreateXxHash3_128_State(const Nan::FunctionCallbackInfo<v8::Value>& info);