#include <unordered_map>
#include <mutex>

class MarketService {
    ...
private:
    DataService* dataService_ = nullptr;

    // Map for fast O(1) lookup of recent CryptoModel by symbol+interval
    std::unordered_map<std::string, std::shared_ptr<models::CryptoModel>> cryptoCache_;
    std::mutex cacheMutex_;
public:
    std::shared_ptr<models::CryptoModel> getCachedCrypto(const std::string& symbol, const std::string& interval) {
        std::string key = symbol + "_" + interval;
        std::lock_guard<std::mutex> lock(cacheMutex_);
        auto it = cryptoCache_.find(key);
        if(it != cryptoCache_.end()) return it->second;
        return nullptr;
    }

    void setCachedCrypto(const std::string& symbol, const std::string& interval, std::shared_ptr<models::CryptoModel> model) {
        std::string key = symbol + "_" + interval;
        std::lock_guard<std::mutex> lock(cacheMutex_);
        cryptoCache_[key] = model;
    }
};

std::shared_ptr<models::CryptoModel> MarketService::getCryptoRange(
    const std::string& symbol,
    const std::string& interval,
    int rangeDays)
{
    if(!dataService_) dataService_ = &DataService::instance();

    // First try cache
    auto cached = getCachedCrypto(symbol, interval);
    if(cached) return cached;

    try {
        auto model = dataService_->getCryptoData(symbol, interval);
        if(model) setCachedCrypto(symbol, interval, model); // store in map
        return model;
    } catch (const std::exception& e) {
        LOG_WARN << "MarketService::getCryptoRange exception: " << e.what();
        return nullptr;
    }
}

