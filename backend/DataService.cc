    #include "DataService.h"
    #include <drogon/utils/Utilities.h>
    #include <trantor/utils/Logger.h>
    #include <trantor/utils/Date.h> // Include for date parsing

    DataService& DataService::instance() {
        static DataService instance;
        return instance;
    }

    void DataService::init() {
        // Initialize persistent HTTP clients
        binanceClient = drogon::HttpClient::newHttpClient("https://api.binance.com");
        alpacaClient = drogon::HttpClient::newHttpClient("https://paper-api.alpaca.markets/v2");

        // --- Read API keys from Drogon's app config ---
        const auto& config = drogon::app().getCustomConfig();  // << FIXED

        if (config.isMember("alpaca_api_key")) {
            alpacaApiKey = config["alpaca_api_key"].asString();
            LOG_INFO << "Loaded Alpaca API Key from secrets.json.";
        } else {
            LOG_WARN << "'alpaca_api_key' not found in config. Alpaca API will fail.";
        }

        if (config.isMember("alpaca_api_secret")) {
            alpacaApiSecret = config["alpaca_api_secret"].asString();
            LOG_INFO << "Loaded Alpaca API Secret from secrets.json.";
        } else {
            LOG_WARN << "'alpaca_api_secret' not found in config. Alpaca API will fail.";
        }
    }


    // --- PUBLIC METHODS ---

    std::shared_ptr<models::CryptoModel> DataService::getCryptoData(const std::string& symbol, const std::string& interval) {
        // 1. Try Binance first
        auto model = getCryptoDataFromBinance(symbol, interval);
        
        if (model) {
            return model; // Success!
        }

        // 2. If Binance fails, log it and try Alpaca
        LOG_WARN << "Binance fetch failed for " << symbol << ". Trying Alpaca as fallback.";
        
        // Note: Alpaca uses different interval strings (e.g., "1Min", "1H")
        // We map common intervals.
        std::string alpacaInterval = interval;
        if (interval == "1m") alpacaInterval = "1Min";
        if (interval == "1h") alpacaInterval = "1H";
        if (interval == "1d") alpacaInterval = "1D";

        model = getCryptoDataFromAlpaca(symbol, alpacaInterval);

        return model; // Return Alpaca's result (which could also be nullptr)
    }


    std::shared_ptr<models::StockModel> DataService::getStockData(const std::string& symbol, const std::string& interval) {
        if (!alpacaClient) {
            LOG_ERROR << "Alpaca client not initialized!";
            return nullptr;
        }

        // --- REAL ALPACA (STOCKS) IMPLEMENTATION ---
        auto req = drogon::HttpRequest::newHttpRequest();
        req->setMethod(drogon::Get);
        // Use the v2 stocks endpoint
        std::string path = "/v2/stocks/" + drogon::utils::urlEncode(symbol) + "/bars";
        req->setPath(path);
        
        req->setParameter("timeframe", interval);
        req->setParameter("limit", "100");

        // Add required Alpaca headers
        req->addHeader("APCA-API-KEY-ID", alpacaApiKey);
        req->addHeader("APCA-API-SECRET-KEY", alpacaApiSecret);

        try {
            auto resultPair = alpacaClient->sendRequest(req, 10.0);
            auto result = resultPair.first;
            auto resp = resultPair.second;

            if (result != drogon::ReqResult::Ok || !resp || resp->statusCode() != drogon::k200OK) {
                LOG_WARN << "Alpaca (Stock) API request failed for " << symbol
                        << " Status: " << (resp ? resp->statusCode() : -1)
                        << " Body: " << (resp ? resp->getBody() : "No response");
                return nullptr;
            }

            auto alpacaJson = json::parse(resp->getBody().data());
            auto model = std::make_shared<models::StockModel>(); // StockModel
            model->symbol = alpacaJson.value("symbol", symbol);
            model->interval = interval;
            if (alpacaJson.contains("bars")) {
                for (const auto &item : alpacaJson["bars"]) {
                    models::Candle candle;
                    if (item["t"].is_string()) {
                        std::string dateString = item["t"].get<std::string>();
                        dateString.replace(dateString.find("T"), 1, " ");
                        if (!dateString.empty() && dateString.back() == 'Z') {
                            dateString.pop_back();
                        }
                        trantor::Date date = trantor::Date::fromDbString(dateString);
                        candle.time = date.secondsSinceEpoch();
                    } else {
                        candle.time = item["t"].get<long long>();
                    }
                    candle.open = item["o"].get<double>();
                    candle.high = item["h"].get<double>();
                    candle.low = item["l"].get<double>();
                    candle.close = item["c"].get<double>();
                    candle.volume = item["v"].get<double>();
                    model->candles.push_back(candle);
                }
            }
            return model;

        }
        catch (const std::exception &e) {
            LOG_ERROR << "Exception in getStockData: " << e.what();
            return nullptr;
        }
    }

    json DataService::searchSymbol(const std::string& query) {
        // This is still a MOCK implementation.
        // A real implementation would query both APIs.
        LOG_WARN << "searchSymbol is using MOCKED data for query: " << query;
        json results = json::array();
        
        if (std::string("BTCUSDT").find(query) != std::string::npos) {
            results.push_back({{"symbol", "BTCUSDT"}, {"name", "Bitcoin"}, {"type", "crypto"}, {"exchange", "Binance/Alpaca"}});
        }
        if (std::string("ETHUSDT").find(query) != std::string::npos) {
            results.push_back({{"symbol", "ETHUSDT"}, {"name", "Ethereum"}, {"type", "crypto"}, {"exchange", "Binance/Alpaca"}});
        }
        if (std::string("AAPL").find(query) != std::string::npos) {
            results.push_back({{"symbol", "AAPL"}, {"name", "Apple Inc."}, {"type", "stock"}, {"exchange", "Alpaca"}});
        }
        return results;
    }


    // --- PRIVATE HELPER METHODS ---

    std::shared_ptr<models::CryptoModel> DataService::getCryptoDataFromBinance(const std::string& symbol, const std::string& interval) {
        if (!binanceClient) {
            LOG_ERROR << "Binance client not initialized!";
            return nullptr;
        }

        auto req = drogon::HttpRequest::newHttpRequest();
        req->setMethod(drogon::Get);
        req->setPath("/api/v3/klines");
        req->setParameter("symbol", symbol);
        req->setParameter("interval", interval);
        req->setParameter("limit", "100");

        try {
            auto resultPair = binanceClient->sendRequest(req, 10.0); // 10s timeout
            auto result = resultPair.first;
            auto resp = resultPair.second;

            if (result != drogon::ReqResult::Ok || !resp || resp->statusCode() != drogon::k200OK) {
                LOG_WARN << "Binance API request failed for " << symbol
                        << " Status: " << (resp ? resp->statusCode() : -1);
                return nullptr;
            }

            auto binanceJson = json::parse(resp->getBody().data());
            auto model = std::make_shared<models::CryptoModel>();
            model->symbol = symbol;
            model->interval = interval;
            model->name = symbol; 
            model->exchange = "Binance"; // Set the exchange

            // Normalize Binance data
            for (const auto& item : binanceJson) {
                models::Candle candle;
                candle.time = item[0].get<long long>() / 1000; // Convert ms to s
                candle.open = std::stod(item[1].get<std::string>());
                candle.high = std::stod(item[2].get<std::string>());
                candle.low = std::stod(item[3].get<std::string>());
                candle.close = std::stod(item[4].get<std::string>());
                candle.volume = std::stod(item[5].get<std::string>());
                model->candles.push_back(candle);
            }
            return model;

        } catch (const std::exception& e) {
            LOG_ERROR << "Exception in getCryptoDataFromBinance: " << e.what();
            return nullptr;
        }
    }

    std::shared_ptr<models::CryptoModel> DataService::getCryptoDataFromAlpaca(const std::string& symbol, const std::string& interval) {
        if (!alpacaClient) {
            LOG_ERROR << "Alpaca client not initialized!";
            return nullptr;
        }

        auto req = drogon::HttpRequest::newHttpRequest();
        req->setMethod(drogon::Get);
        
        // Alpaca crypto symbols might be "BTC/USD"
        // We'll try to map common symbols
        std::string alpacaSymbol = symbol;
        if(symbol == "BTCUSDT") alpacaSymbol = "BTC/USD";
        if(symbol == "ETHUSDT") alpacaSymbol = "ETH/USD";
        
        std::string path = "/v2/crypto/" + drogon::utils::urlEncode(alpacaSymbol) + "/bars";
        req->setPath(path);
        
        req->setParameter("timeframe", interval);
        req->setParameter("limit", "100");

        // Add required Alpaca headers
        req->addHeader("APCA-API-KEY-ID", alpacaApiKey);
        req->addHeader("APCA-API-SECRET-KEY", alpacaApiSecret);

        try {
            auto resultPair = alpacaClient->sendRequest(req, 10.0);
            auto result = resultPair.first;
            auto resp = resultPair.second;

            if (result != drogon::ReqResult::Ok || !resp || resp->statusCode() != drogon::k200OK) {
                LOG_WARN << "Alpaca (Crypto) API request failed for " << symbol
                        << " Status: " << (resp ? resp->statusCode() : -1)
                        << " Body: " << (resp ? resp->getBody() : "No response");
                return nullptr;
            }

            auto alpacaJson = json::parse(resp->getBody().data());
            auto model = std::make_shared<models::CryptoModel>(); // CryptoModel
            model->symbol = alpacaJson.value("symbol", symbol);
            model->interval = interval;
            model->name = symbol;
            model->exchange = "Alpaca (Crypto)"; // Set the exchange

            // Normalize Alpaca data
            if(alpacaJson.contains("bars")) {
                for (const auto& item : alpacaJson["bars"]) {
                    models::Candle candle;
                    if (item["t"].is_string()) {
                        std::string dateString = item["t"].get<std::string>();
                        dateString.replace(dateString.find("T"), 1, " ");
                        if (!dateString.empty() && dateString.back() == 'Z') {
                            dateString.pop_back();
                        }
                        trantor::Date date = trantor::Date::fromDbString(dateString);
                        candle.time = date.secondsSinceEpoch();
                    } else {
                        candle.time = item["t"].get<long long>();
                    }
                    candle.open = item["o"].get<double>();
                    candle.high = item["h"].get<double>();
                    candle.low = item["l"].get<double>();
                    candle.close = item["c"].get<double>();
                    candle.volume = item["v"].get<double>();
                    model->candles.push_back(candle);
                }
            }
            return model;

        } catch (const std::exception& e) {
            LOG_ERROR << "Exception in getCryptoDataFromAlpaca: " << e.what();
            return nullptr;
        }
    }