#ifndef K_EV_H_
#define K_EV_H_

namespace K {
  typedef void (*evCb)(Local<Object>);
  struct Ev { map<string, vector<CopyablePersistentTraits<Function>::CopyablePersistent>> _cb; map<string, vector<evCb>> cb; } ev;
  class EV {
    public:
      static void main(Local<Object> exports) {
        Ev *ev = new Ev;
        NODE_SET_METHOD(exports, "evOn", EV::_evOn);
        NODE_SET_METHOD(exports, "evUp", EV::_evUp);
      }
      static void evOn(string k, evCb cb) {
        ev.cb[k].push_back(cb);
      };
      static void evUp(string k) {
        Isolate* isolate = Isolate::GetCurrent();
        Local<Object> o = Object::New(isolate);
        if (ev.cb.find(k) != ev.cb.end()) {
          for (vector<evCb>::iterator cb = ev.cb[k].begin(); cb != ev.cb[k].end(); ++cb)
            (*cb)(o);
        }
        if (ev._cb.find(k) != ev._cb.end()) {
          Local<Value> argv[] = {o};
          for (vector<CopyablePersistentTraits<Function>::CopyablePersistent>::iterator _cb = ev._cb[k].begin(); _cb != ev._cb[k].end(); ++_cb)
            Local<Function>::New(isolate, *_cb)->Call(isolate->GetCurrentContext()->Global(), 1, argv);
        }
      };
      static void evUp(string k, Local<Object> o) {
        if (ev.cb.find(k) != ev.cb.end()) {
          for (vector<evCb>::iterator cb = ev.cb[k].begin(); cb != ev.cb[k].end(); ++cb)
            (*cb)(o);
        }
        if (ev._cb.find(k) != ev._cb.end()) {
          Isolate* isolate = Isolate::GetCurrent();
          Local<Value> argv[] = {o};
          for (vector<CopyablePersistentTraits<Function>::CopyablePersistent>::iterator _cb = ev._cb[k].begin(); _cb != ev._cb[k].end(); ++_cb)
            Local<Function>::New(isolate, *_cb)->Call(isolate->GetCurrentContext()->Global(), 1, argv);
        }
      };
      static void _evOn(const FunctionCallbackInfo<Value> &args) {
        Isolate* isolate = args.GetIsolate();
        HandleScope scope(isolate);
        string k = string(*String::Utf8Value(args[0]->ToString()));
        Persistent<Function> _cb;
        _cb.Reset(isolate, Local<Function>::Cast(args[1]));
        ev._cb[k].push_back(_cb);
      };
      static void _evUp(const FunctionCallbackInfo<Value> &args) {
        Isolate* isolate = args.GetIsolate();
        HandleScope scope(isolate);
        string k = string(*String::Utf8Value(args[0]->ToString()));
        if (ev._cb.find(k) != ev._cb.end()) {
          Local<Value> argv[] = {args[1]};
          for (vector<CopyablePersistentTraits<Function>::CopyablePersistent>::iterator _cb = ev._cb[k].begin(); _cb != ev._cb[k].end(); ++_cb)
            Local<Function>::New(isolate, *_cb)->Call(isolate->GetCurrentContext()->Global(), 1, argv);
        }
        if (ev.cb.find(k) != ev.cb.end()) {
          for (vector<evCb>::iterator cb = ev.cb[k].begin(); cb != ev.cb[k].end(); ++cb)
            (*cb)(args[1]->IsUndefined() ? Object::New(isolate) : args[1]->ToObject());
        }
      };
  };
}

#endif
