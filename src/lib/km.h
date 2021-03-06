#ifndef K_KM_H_
#define K_KM_H_

namespace K {
  enum class mExchange: unsigned int { Null, HitBtc, OkCoin, Coinbase, Bitfinex, Korbit, Poloniex };
  vector<string> mCurrency = {
    "BTC", "LTC", "EUR", "GBP", "CNY", "CAD", "ETH", "ETC", "BFX", "RRT", "ZEC", "BCN", "DASH", "DOGE",
    "DSH", "EMC", "FCN", "LSK", "NXT", "QCN", "SDB", "SCB", "STEEM", "XDN", "XEM", "XMR", "ARDR", "WAVES",
    "BTU", "MAID", "AMP", "XRP", "KRW", "IOT", "BCY", "BELA", "BLK", "BTCD", "BTM", "BTS", "BURST", "CLAM",
    "DCR", "DGB", "EMC2", "EXP", "FCT", "FLDC", "FLO", "GAME", "GNO", "GNT", "GRC", "HUC", "LBC", "NAUT",
    "NAV", "NEOS", "NMC", "NOTE", "NXC", "OMNI", "PASC", "PINK", "POT", "PPC", "RADS", "REP", "RIC", "SBD",
    "SC", "SJCX", "STR", "STRAT", "SYS", "VIA", "VRC", "VTC", "XBC", "XCP", "XPM", "XVC", "USD", "USDT",
    "EOS", "SAN", "OMG", "PAY" };
  enum class mGatewayType: unsigned int { MarketData, OrderEntry, Position };
  enum class mConnectivityStatus: unsigned int { Connected, Disconnected };
  enum class mSide: unsigned int { Bid, Ask, Unknown };
  enum class mORS: unsigned int { New, Working, Complete, Cancelled };
  enum class mPingAt: unsigned int { BothSides, BidSide, AskSide, DepletedSide, DepletedBidSide, DepletedAskSide, StopPings };
  enum class mPongAt: unsigned int { ShortPingFair, LongPingFair, ShortPingAggressive, LongPingAggressive };
  enum class mQuotingMode: unsigned int { Top, Mid, Join, InverseJoin, InverseTop, PingPong, Boomerang, AK47, HamelinRat, Depth };
  enum class mFairValueModel: unsigned int { BBO, wBBO };
  enum class mAutoPositionMode: unsigned int { Manual, EWMA_LS, EWMA_LMS };
  enum class mAPR: unsigned int { Off, Size, SizeWidth };
  enum class mSOP: unsigned int { Off, x2trades, x3trades, x2Size, x3Size, x2tradesSize, x3tradesSize };
  enum class mSTDEV: unsigned int{ Off, OnFV, OnFVAPROff, OnTops, OnTopsAPROff, OnTop, OnTopAPROff };
  enum class uiBIT: unsigned char { MSG = '-', SNAP = '=' };
  enum class uiTXT: unsigned char {
    FairValue = 'a',
    Quote = 'b',
    ActiveSubscription = 'c',
    ActiveState = 'd',
    MarketData = 'e',
    QuotingParametersChange = 'f',
    SafetySettings = 'g',
    Product = 'h',
    OrderStatusReports = 'i',
    ProductAdvertisement = 'j',
    ApplicationState = 'k',
    Notepad = 'l',
    ToggleConfigs = 'm',
    Position = 'n',
    ExchangeConnectivity = 'o',
    SubmitNewOrder = 'p',
    CancelOrder = 'q',
    MarketTrade = 'r',
    Trades = 's',
    ExternalValuation = 't',
    QuoteStatus = 'u',
    TargetBasePosition = 'v',
    TradeSafetyValue = 'w',
    CancelAllOrders = 'x',
    CleanAllClosedOrders = 'y',
    CleanAllOrders = 'z',
    CleanTrade = 'A',
    TradesChart = 'B',
    WalletChart = 'C',
    EWMAChart = 'D'
  };
  int FN::S2mC(string k) {
    k = FN::S2u(k);
    for (unsigned i=0; i<mCurrency.size(); ++i) if (mCurrency[i] == k) return i;
    cout << FN::uiT() << "Errrror: Use of missing \"" << k << "\" currency." << endl; exit(1);
  };
  Persistent<Object> qpRepo;
  string cFname;
  json cfRepo;
  json pkRepo;
  sqlite3* db;
  string dbFpath;
  int sqlite3_open(string f, sqlite3** db);
  int sqlite3_exec(sqlite3** db, string q);
  uWS::Hub hub(0, true);
  uv_check_t loop;
  uv_timer_t uiD_;
  Persistent<Function> noop;
  typedef Local<Value> (*uiCb)(Local<Value>);
  struct uiSess { map<string, Persistent<Function>> _cb; map<string, uiCb> cb; map<uiTXT, vector<CopyablePersistentTraits<Object>::CopyablePersistent>> D; int u = 0; };
  uWS::Group<uWS::SERVER> *uiGroup = hub.createGroup<uWS::SERVER>(uWS::PERMESSAGE_DEFLATE);
  int iOSR60 = 0;
  bool uiOPT = true;
  unsigned long uiMDT = 0;
  unsigned long uiDDT = 0;
  string uiNOTE = "";
  string uiNK64 = "";
  Persistent<Function> socket_;
  Persistent<Object> _app_state;
}

#endif
