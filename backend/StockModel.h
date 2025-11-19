#pragma once

#include "DashboardModel.h"

namespace models {

/**
 * @brief Represents a Stock asset, inheriting from DashboardModel.
 */
struct StockModel : public DashboardModel {
    std::string exchange = "Alpaca (Stocks)";

    // Constructor to set type
    StockModel() {
        type = "stock";
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