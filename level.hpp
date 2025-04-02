// level.hpp
// A single orderbook level on some side
#pragma once

#include "shared.hpp"

class OrderbookLevel {
  PriceType price;
  QuantityType total_volume;
  OrderListType orders;
public:
  OrderbookLevel(PriceType price) : price(price), total_volume(0) {}

  inline void add_order(const Order &order) {
    orders.push_back(order);
    total_volume += order.quantity;
  }

  void remove_order(IdType order_id) {
    orders.remove_if([&](const Order &order) {
      if (order.id == order_id) {
        total_volume -= order.quantity;
        return true;
      }
      return false;
    });
  }

  inline uint32_t total_quantity() const {
    return total_volume;
  }

  inline PriceType get_price() const {
    return price;
  }

  // overload existing functions to make out of the box implementation work
  // push back works with orders
  inline void push_back(const Order &order) { add_order(order); }
  // erase erases by iterator
  auto erase(const OrderListType::iterator &it) {
    if (it == orders.end()) {
      return orders.end();
    }
    total_volume -= it->quantity;
    return orders.erase(it);
  }
  
  // size works with orders
  inline QuantityType size() const { return orders.size(); }
  inline bool empty() const { return orders.empty(); }


  // iterator access
  inline auto begin() { return orders.begin(); }
  inline auto end() { return orders.end(); }
  inline auto cbegin() const { return orders.cbegin(); }
  inline auto cend() const { return orders.cend(); }
  
  // get orders
  inline const OrderListType &get_orders() const { return orders; }
};

