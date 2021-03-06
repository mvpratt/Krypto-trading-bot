#ifndef K_GW_H_
#define K_GW_H_

namespace K {
  class Gw {
    public:
      static Gw *E(mExchange e);
      mExchange exchange = mExchange::Null;
      double makeFee = 0;
      double takeFee = 0;
      double minTick = 0;
      double minSize = 0;
      string symbol = "";
      string target = "";
      string apikey = "";
      string secret = "";
      string user = "";
      string pass = "";
      string http = "";
      string ws = "";
      string wS = "";
      int quote = 0;
      int base = 0;
      virtual void fetch() = 0;
      virtual void pos() = 0;
  };
  uv_timer_t gwPos_;
  bool gwAutoStart = false;
  bool gwState = false;
  mConnectivityStatus gwConn = mConnectivityStatus::Disconnected;
  mConnectivityStatus gwMDConn = mConnectivityStatus::Disconnected;
  mConnectivityStatus gwEOConn = mConnectivityStatus::Disconnected;
  Gw* gw;
  class GW {
    public:
      static void main(Local<Object> exports) {
        gw = Gw::E(CF::cfExchange());
        gw->base = CF::cfBase();
        gw->quote = CF::cfQuote();
        gw->fetch();
        if (!gw->minTick) { cout << FN::uiT() << "Errrror: Unable to match TradedPair to " << CF::cfString("EXCHANGE") << " symbol \"" << gw->symbol << "\"." << endl; exit(1); }
        else { cout << FN::uiT() << "GW " << CF::cfString("EXCHANGE") << " allows client IP." << endl; }
        gwAutoStart = "auto" == CF::cfString("BotIdentifier").substr(0,4);
        cout << FN::uiT() << "GW " << setprecision(8) << fixed << CF::cfString("EXCHANGE") << ":" << endl << "- autoBot: " << (gwAutoStart ? "yes" : "no") << endl << "- pair: " << gw->symbol << endl << "- minTick: " << gw->minTick << endl << "- minSize: " << gw->minSize << endl << "- makeFee: " << gw->makeFee << endl << "- takeFee: " << gw->takeFee << endl;
        if (uv_timer_init(uv_default_loop(), &gwPos_)) { cout << FN::uiT() << "Errrror: GW gwPos_ init timer failed." << endl; exit(1); }
        gwPos_.data = gw;
        if (uv_timer_start(&gwPos_, gwPos__, 0, 15000)) { cout << FN::uiT() << "Errrror: UV gwPos_ start timer failed." << endl; exit(1); }
        EV::evOn("GatewayMarketConnect", [](Local<Object> c) {
          gwCon__(mGatewayType::MarketData, (mConnectivityStatus)c->NumberValue());
        });
        EV::evOn("GatewayOrderConnect", [](Local<Object> c) {
          gwCon__(mGatewayType::OrderEntry, (mConnectivityStatus)c->NumberValue());
        });
        UI::uiSnap(uiTXT::ProductAdvertisement, &onSnapProduct);
        UI::uiSnap(uiTXT::ExchangeConnectivity, &onSnapStatus);
        UI::uiSnap(uiTXT::ActiveState, &onSnapState);
        UI::uiHand(uiTXT::ActiveState, &onHandState);
        NODE_SET_METHOD(exports, "gwMakeFee", GW::_gwMakeFee);
        NODE_SET_METHOD(exports, "gwTakeFee", GW::_gwTakeFee);
        NODE_SET_METHOD(exports, "gwMinTick", GW::_gwMinTick);
        NODE_SET_METHOD(exports, "gwMinSize", GW::_gwMinSize);
        NODE_SET_METHOD(exports, "gwExchange", GW::_gwExchange);
        NODE_SET_METHOD(exports, "gwSymbol", GW::_gwSymbol);
      };
      static void gwPos__(uv_timer_t *handle) {
        Gw* gw = (Gw*) handle->data;
        gw->pos();
      };
      static void gwPosUp(double amount, double held, int currency) {
        Isolate* isolate = Isolate::GetCurrent();
        HandleScope scope(isolate);
        Local<Object> o = Object::New(isolate);
        o->Set(FN::v8S("amount"), Number::New(isolate, amount));
        o->Set(FN::v8S("heldAmount"), Number::New(isolate, held));
        o->Set(FN::v8S("currency"), Number::New(isolate, currency));
        EV::evUp("PositionGateway", o);
      };
    private:
      static Local<Value> onSnapProduct(Local<Value> z) {
        Isolate* isolate = Isolate::GetCurrent();
        Local<Array> k_ = Array::New(isolate);
        Local<Object> k = Object::New(isolate);
        k->Set(FN::v8S("exchange"), Number::New(isolate, (double)gw->exchange));
        Local<Object> o = Object::New(isolate);
        o->Set(FN::v8S("base"), Number::New(isolate, (double)gw->base));
        o->Set(FN::v8S("quote"), Number::New(isolate, (double)gw->quote));
        k->Set(FN::v8S("pair"), o);
        k->Set(FN::v8S("environment"), FN::v8S(isolate, CF::cfString("BotIdentifier").substr(gwAutoStart?4:0)));
        k->Set(FN::v8S("matryoshka"), FN::v8S(isolate, CF::cfString("MatryoshkaUrl")));
        k->Set(FN::v8S("homepage"), FN::v8S(isolate, CF::cfPKString("homepage")));
        k->Set(FN::v8S("minTick"), Number::New(isolate, gw->minTick));
        k_->Set(0, k);
        return k_;
      };
      static Local<Value> onSnapStatus(Local<Value> z) {
        Isolate* isolate = Isolate::GetCurrent();
        Local<Array> k = Array::New(isolate);
        k->Set(0, Number::New(isolate, (double)gwConn));
        return k;
      };
      static Local<Value> onSnapState(Local<Value> z) {
        Isolate* isolate = Isolate::GetCurrent();
        Local<Array> k = Array::New(isolate);
        k->Set(0, Boolean::New(isolate, gwState));
        return k;
      };
      static Local<Value> onHandState(Local<Value> s) {
        Isolate* isolate = Isolate::GetCurrent();
        if (s->BooleanValue() != gwAutoStart) {
          gwAutoStart = s->BooleanValue();
          gwUpState();
          Local<Object> o = Object::New(isolate);
          o->Set(FN::v8S("state"), Boolean::New(isolate, gwState));
          o->Set(FN::v8S("status"), Number::New(isolate, (double)gwConn));
          EV::evUp("ExchangeConnect", o);
        }
        return (Local<Value>)Undefined(isolate);
      };
      static void gwCon__(mGatewayType gwT, mConnectivityStatus gwS) {
        if (gwT == mGatewayType::MarketData) {
          if (gwMDConn == gwS) return;
          gwMDConn = gwS;
        } else if (gwT == mGatewayType::OrderEntry) {
          if (gwEOConn == gwS) return;
          gwEOConn = gwS;
        }
        gwConn = gwMDConn == mConnectivityStatus::Connected && gwEOConn == mConnectivityStatus::Connected
          ? mConnectivityStatus::Connected : mConnectivityStatus::Disconnected;
        gwUpState();
        Isolate* isolate = Isolate::GetCurrent();
        Local<Object> o = Object::New(isolate);
        o->Set(FN::v8S("state"), Boolean::New(isolate, gwState));
        o->Set(FN::v8S("status"), Number::New(isolate, (double)gwConn));
        EV::evUp("ExchangeConnect", o);
        UI::uiSend(isolate, uiTXT::ExchangeConnectivity, Number::New(isolate, (double)gwConn)->ToObject());
      };
      static void gwUpState() {
        Isolate* isolate = Isolate::GetCurrent();
        bool newMode = gwConn != mConnectivityStatus::Connected ? false : gwAutoStart;
        if (newMode != gwState) {
          gwState = newMode;
          cout << FN::uiT() << "GW " << CF::cfString("EXCHANGE") << " Changed quoting state to " << (gwState ? "Enabled" : "Disabled") << "." << endl;
          UI::uiSend(isolate, uiTXT::ActiveState, Boolean::New(isolate, gwState)->ToObject());
        }
      }
      static void _gwMakeFee(const FunctionCallbackInfo<Value> &args) {
        Isolate* isolate = args.GetIsolate();
        HandleScope scope(isolate);
        args.GetReturnValue().Set(Number::New(isolate, gw->makeFee));
      };
      static void _gwTakeFee(const FunctionCallbackInfo<Value> &args) {
        Isolate* isolate = args.GetIsolate();
        HandleScope scope(isolate);
        args.GetReturnValue().Set(Number::New(isolate, gw->takeFee));
      };
      static void _gwMinTick(const FunctionCallbackInfo<Value> &args) {
        Isolate* isolate = args.GetIsolate();
        HandleScope scope(isolate);
        args.GetReturnValue().Set(Number::New(isolate, gw->minTick));
      };
      static void _gwMinSize(const FunctionCallbackInfo<Value> &args) {
        Isolate* isolate = args.GetIsolate();
        HandleScope scope(isolate);
        args.GetReturnValue().Set(Number::New(isolate, gw->minSize));
      };
      static void _gwExchange(const FunctionCallbackInfo<Value> &args) {
        Isolate* isolate = args.GetIsolate();
        HandleScope scope(isolate);
        args.GetReturnValue().Set(Number::New(isolate, (double)gw->exchange));
      };
      static void _gwSymbol(const FunctionCallbackInfo<Value> &args) {
        Isolate* isolate = args.GetIsolate();
        HandleScope scope(isolate);
        args.GetReturnValue().Set(FN::v8S(isolate, gw->symbol));
      };
  };
  class GwNull: public Gw {
    public:
      void fetch() {
        symbol = string(mCurrency[base]).append("_").append(mCurrency[quote]);
        minTick = 0.01;
        minSize = 0.01;
      };
      void pos() {
        GW::gwPosUp(500, 50, base);
        GW::gwPosUp(500, 50, quote);
      };
  };
  class GwOkCoin: public Gw {
    public:
      void fetch() {
        exchange = mExchange::OkCoin;
        symbol = FN::S2l(string(mCurrency[base]).append("_").append(mCurrency[quote]));
        target = CF::cfString("OkCoinOrderDestination");
        apikey = CF::cfString("OkCoinApiKey");
        secret = CF::cfString("OkCoinSecretKey");
        http = CF::cfString("OkCoinHttpUrl");
        ws = CF::cfString("OkCoinWsUrl");
        minTick = "btc" == symbol.substr(0,3) ? 0.01 : 0.001;
        minSize = 0.01;
      };
      void pos() {
        string p = string("api_key=").append(apikey);
        json k = FN::wJet(string(http).append("userinfo.do"), p.append("&sign=").append(FN::oMd5(string(p).append("&secret_key=").append(secret))));
        if (k["result"] != true) cout << FN::uiT() << "GW " << CF::cfString("EXCHANGE") << " Unable to read wallet data positions." << endl;
        else for (json::iterator it = k["info"]["funds"]["free"].begin(); it != k["info"]["funds"]["free"].end(); ++it)
          if (symbol.find(it.key()) != string::npos)
            GW::gwPosUp(stod(k["info"]["funds"]["free"][it.key()].get<string>()), stod(k["info"]["funds"]["freezed"][it.key()].get<string>()), FN::S2mC(it.key()));
      };
  };
  class GwCoinbase: public Gw {
    public:
      void fetch() {
        exchange = mExchange::Coinbase;
        symbol = string(mCurrency[base]).append("-").append(mCurrency[quote]);
        target = CF::cfString("CoinbaseOrderDestination");
        apikey = CF::cfString("CoinbaseApiKey");
        secret = CF::cfString("CoinbaseSecret");
        pass = CF::cfString("CoinbasePassphrase");
        http = CF::cfString("CoinbaseRestUrl");
        ws = CF::cfString("CoinbaseWebsocketUrl");
        json k = FN::wJet(string(http).append("/products/").append(symbol));
        if (k.find("quote_increment") != k.end()) {
          minTick = stod(k["quote_increment"].get<string>());
          minSize = stod(k["base_min_size"].get<string>());
        }
      };
      void pos() {
        unsigned long t = FN::T() / 1000;
        string p;
        B64::Decode(secret, &p);
        B64::Encode(FN::oHmac256(string(to_string(t)).append("GET/accounts"), p), &p);
        json k = FN::wJet(string(http).append("/accounts"), to_string(t), apikey, p, pass);
        if (k.find("message") != k.end()) cout << FN::uiT() << "GW " << CF::cfString("EXCHANGE") << " Warning " << k["message"] << endl;
        else for (json::iterator it = k.begin(); it != k.end(); ++it)
          if ((*it)["currency"] == mCurrency[base] || (*it)["currency"] == mCurrency[quote])
            GW::gwPosUp(stod((*it)["available"].get<string>()), stod((*it)["hold"].get<string>()), FN::S2mC((*it)["currency"]));
      };
  };
  class GwBitfinex: public Gw {
    public:
      void fetch() {
        exchange = mExchange::Bitfinex;
        symbol = FN::S2l(string(mCurrency[base]).append(mCurrency[quote]));
        target = CF::cfString("BitfinexOrderDestination");
        apikey = CF::cfString("BitfinexKey");
        secret = CF::cfString("BitfinexSecret");
        http = CF::cfString("BitfinexHttpUrl");
        ws = CF::cfString("BitfinexWebsocketUrl");
        json k = FN::wJet(string(http).append("/pubticker/").append(symbol));
        if (k.find("last_price") != k.end()) {
          string k_ = to_string(stod(k["last_price"].get<string>()) / 10000);
          unsigned int i = 0;
          for (string::iterator it=k_.begin(); it!=k_.end(); ++it)
            if (*it == '0' || *it == '.') i++; else break;
          stringstream os(string("1e-").append(to_string(i)));
          os >> minTick;
          minSize = 0.01;
        }
      };
      void pos() {
        string p;
        B64::Encode(string("{\"request\":\"/v1/balances\",\"nonce\":\"").append(to_string(FN::T()*1000)).append("\"}"), &p);
        json k = FN::wJet(string(http).append("/balances"), p, apikey, FN::oHmac384(p, secret), true);
        if (k.find("message") != k.end()) {
          if (k["message"]=="Nonce is too small.") pos();
          else cout << FN::uiT() << "GW " << CF::cfString("EXCHANGE") << " Warning " << k["message"] << endl;
        } else for (json::iterator it = k.begin(); it != k.end(); ++it) if ((*it)["type"] == "exchange")
          GW::gwPosUp(stod((*it)["available"].get<string>()), stod((*it)["amount"].get<string>()) - stod((*it)["available"].get<string>()), FN::S2mC((*it)["currency"]));
      };
  };
  class GwKorbit: public Gw {
    public:
      unsigned long token_time_ = 0;
      string token_ = "";
      string token_refresh_ = "";
      void fetch() {
        exchange = mExchange::Korbit;
        symbol = FN::S2l(string(mCurrency[base]).append("_").append(mCurrency[quote]));
        target = CF::cfString("KorbitOrderDestination");
        apikey = CF::cfString("KorbitApiKey");
        secret = CF::cfString("KorbitSecretKey");
        user = CF::cfString("KorbitUsername");
        pass = CF::cfString("KorbitPassword");
        http = CF::cfString("KorbitHttpUrl");
        json k = FN::wJet(string(http).append("/constants"));
        if (k.find(symbol.substr(0,3).append("TickSize")) != k.end()) {
          minTick = k[symbol.substr(0,3).append("TickSize")];
          minSize = 0.015;
        }
      };
      void pos() {
        if (!token_time_) {
          json k = FN::wJet(string(http).append("/oauth2/access_token"), string("client_id=").append(apikey).append("&client_secret=").append(secret).append("&username=").append(user).append("&password=").append(pass).append("&grant_type=password"), token_, "", false, true);
          token_time_ = (k["expires_in"].get<int>() * 1000) + FN::T();
          token_ = k["access_token"];
          token_refresh_ = k["refresh_token"];
          cout << FN::uiT() << "GW " << CF::cfString("EXCHANGE") << " Authentication successful, new token expires at " << to_string(token_time_) << "." << endl;
        } else if (FN::T()+60 > token_time_) {
          json k = FN::wJet(string(http).append("/oauth2/access_token"), string("client_id=").append(apikey).append("&client_secret=").append(secret).append("&refresh_token=").append(token_refresh_).append("&grant_type=refresh_token"), token_, "", false, true);
          token_time_ = (k["expires_in"].get<int>() * 1000) + FN::T();
          token_ = k["access_token"];
          token_refresh_ = k["refresh_token"];
          cout << FN::uiT() << "GW " << CF::cfString("EXCHANGE") << " Authentication refresh successful, new token expires at " << to_string(token_time_) << "." << endl;
        }
        json k = FN::wJet(string(http).append("/user/wallet?currency_pair=").append(symbol), token_, true);
        map<string, vector<double>> wallet;
        for (json::iterator it = k["available"].begin(); it != k["available"].end(); ++it) wallet[(*it)["currency"].get<string>()].push_back(stod((*it)["value"].get<string>()));
        for (json::iterator it = k["pendingOrders"].begin(); it != k["pendingOrders"].end(); ++it) wallet[(*it)["currency"].get<string>()].push_back(stod((*it)["value"].get<string>()));
        for (map<string, vector<double>>::iterator it=wallet.begin(); it!=wallet.end(); ++it) {
          double amount; double held;
          for (unsigned i=0; i<it->second.size(); ++i) if (i==0) amount = it->second[i]; else if (i==1) held = it->second[i];
          GW::gwPosUp(amount, held, FN::S2mC(it->first));
        }
      };
  };
  class GwHitBtc: public Gw {
    public:
      void fetch() {
        exchange = mExchange::HitBtc;
        symbol = string(mCurrency[base]).append(mCurrency[quote]);
        target = CF::cfString("HitBtcOrderDestination");
        apikey = CF::cfString("HitBtcApiKey");
        secret = CF::cfString("HitBtcSecret");
        http = CF::cfString("HitBtcPullUrl");
        ws = CF::cfString("HitBtcOrderEntryUrl");
        wS = CF::cfString("HitBtcMarketDataUrl");
        json k = FN::wJet(string(http).append("/api/1/public/symbols"));
        if (k.find("symbols") != k.end())
          for (json::iterator it = k["symbols"].begin(); it != k["symbols"].end(); ++it)
            if ((*it)["symbol"] == symbol) {
              minTick = stod((*it)["step"].get<string>());
              minSize = stod((*it)["lot"].get<string>());
              break;
            }
      };
      void pos() {
        if (http.find("demo") == string::npos) {
          string p = string("apikey=").append(apikey).append("&nonce=").append(to_string(FN::T()));
          json k = FN::wJet(string(http).append("/api/1/trading/balance?").append(p), p, FN::oHmac512(string("/api/1/trading/balance?").append(p), secret));
          if (k.find("balance") == k.end()) cout << FN::uiT() << "GW " << CF::cfString("EXCHANGE") << " Unable to read wallet data positions." << endl;
          else for (json::iterator it = k["balance"].begin(); it != k["balance"].end(); ++it)
            if ((*it)["currency_code"] == mCurrency[base] || (*it)["currency_code"] == mCurrency[quote])
              GW::gwPosUp((*it)["cash"], (*it)["reserved"], FN::S2mC((*it)["currency_code"]));
        } else {
          GW::gwPosUp(500, 50, base);
          GW::gwPosUp(500, 50, quote);
        }
      };
  };
  class GwPoloniex: public Gw {
    public:
      void fetch() {
        exchange = mExchange::Poloniex;
        symbol = string(mCurrency[quote]).append("_").append(mCurrency[base]);
        string target = CF::cfString("PoloniexOrderDestination");
        apikey = CF::cfString("PoloniexApiKey");
        secret = CF::cfString("PoloniexSecretKey");
        http = CF::cfString("PoloniexHttpUrl");
        ws = CF::cfString("PoloniexWebsocketUrl");
        json k = FN::wJet(string(http).append("/public?command=returnTicker"));
        if (k.find(symbol) != k.end()) {
          istringstream os(string("1e-").append(to_string(6-k[symbol]["last"].get<string>().find("."))));
          os >> minTick;
          minSize = 0.01;
        }
      };
      void pos() {
        string p = string("command=returnCompleteBalances&nonce=").append(to_string(FN::T()));
        json k = FN::wJet(string(http).append("/tradingApi"), p, apikey, FN::oHmac512(p, secret));
        if (k.find(mCurrency[base]) == k.end()) cout << FN::uiT() << "GW " << CF::cfString("EXCHANGE") << " Unable to read wallet data positions." << endl;
        else for (json::iterator it = k.begin(); it != k.end(); ++it)
          if (it.key() == mCurrency[base] || it.key() == mCurrency[quote])
            GW::gwPosUp(stod(k[it.key()]["available"].get<string>()), stod(k[it.key()]["onOrders"].get<string>()), FN::S2mC(it.key()));
      };
  };
  Gw *Gw::E(mExchange e) {
    if (e == mExchange::Null) return new GwNull;
    else if (e == mExchange::OkCoin) return new GwOkCoin;
    else if (e == mExchange::Coinbase) return new GwCoinbase;
    else if (e == mExchange::Bitfinex) return new GwBitfinex;
    else if (e == mExchange::Korbit) return new GwKorbit;
    else if (e == mExchange::HitBtc) return new GwHitBtc;
    else if (e == mExchange::Poloniex) return new GwPoloniex;
    cout << FN::uiT() << "Errrror: No gateway provided for exchange \"" << CF::cfString("EXCHANGE") << "\"." << endl;
    exit(1);
  };
}

#endif
