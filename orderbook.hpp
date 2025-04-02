// orderbook.hpp
#pragma once

#include <list>
#include <map>
#include <memory>

#include "level.hpp"
#include "shared.hpp"

struct Orderbook {
  std::map<PriceType, OrderbookLevel, std::greater<PriceType>> buyOrders;
  std::map<PriceType, OrderbookLevel, std::less<PriceType>> sellOrders;

  // push back a new order
  void push_back(const Order &order) {
    if (order.side == Side::BUY) {
      DEBUG("BUY Pushing back order: " << order.id << " " << order.price << " "
                                   << order.quantity << " " << (int)order.side);
      if (buyOrders.find(order.price) == buyOrders.end()) {
        buyOrders.emplace(order.price, OrderbookLevel(order.price));
      }
      auto itr = buyOrders.find(order.price);
      itr->second.push_back(order);
    } else {
      DEBUG("SELL Pushing back order: " << order.id << " " << order.price << " "
                                   << order.quantity << " " << (int)order.side);
      if (sellOrders.find(order.price) == sellOrders.end()) {
        sellOrders.emplace(order.price, OrderbookLevel(order.price));
      }
      auto itr = sellOrders.find(order.price);
      itr->second.push_back(order);
    }
  }
  //std::map<PriceType, std::list<Order>, std::greater<PriceType>> buyOrders;
  //std::map<PriceType, std::list<Order>> sellOrders;
};

