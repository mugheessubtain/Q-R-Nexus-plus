#pragma once

#include "DashboardModel.h"

namespace models {

/**
 * @brief Represents a Cryptocurrency asset, inheriting from DashboardModel.
 */
struct CryptoModel : public DashboardModel {
    std::string exchange; // No longer hardcoded to Binance

    // Constructor to set type
    CryptoModel() {
        type = "crypto";
    }

    /**
     * @brief Overrides base toJson to add the 'exchange' field.
     */
    json toJson() const override {
        // Call base class implementation
        json j = DashboardModel::toJson();
        
        // Add derived class fields
        j["exchange"] = exchange;
        
        return j;
    }
};

} // namespace models