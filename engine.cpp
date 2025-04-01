#include "engine.hpp"
#include <functional>
#include <optional>
#include <stdexcept>

// This is an example correct implementation
// It is INTENTIONALLY suboptimal
// You are encouraged to rewrite as much or as little as you'd like

// Templated helper to process matching orders.
// The Condition predicate takes the price level and the incoming order price
// and returns whether the level qualifies.
template <typename OrderMap, typename Condition>
uint32_t process_orders(Order &order, OrderMap &ordersMap, Condition cond) {
  uint32_t matchCount = 0;
  auto it = ordersMap.begin();
  while (it != ordersMap.end() && order.quantity > 0 &&
         (it->first == order.price || cond(it->first, order.price))) {
    auto &ordersAtPrice = it->second;
    for (auto orderIt = ordersAtPrice.begin();
         orderIt != ordersAtPrice.end() && order.quantity > 0;) {
      QuantityType trade = std::min(order.quantity, orderIt->second.quantity);
      order.quantity -= trade;
      orderIt->second.quantity -= trade;
      ++matchCount;
      if (orderIt->second.quantity == 0)
        orderIt = ordersAtPrice.erase(orderIt);
      else
        ++orderIt;
    }
    if (ordersAtPrice.empty())
      it = ordersMap.erase(it);
    else
      ++it;
  }
  return matchCount;
}

uint32_t match_order(Orderbook &orderbook, const Order &incoming) {
  uint32_t matchCount = 0;
  Order order = incoming; // Create a copy to modify the quantity

  if (order.side == Side::BUY) {
    // For a BUY, match with sell orders priced at or below the order's price.
    matchCount = process_orders(order, orderbook.sellOrders, std::less<>());
    if (order.quantity > 0)
      orderbook.buyOrders[order.price].insert({order.id, order});
  } else { // Side::SELL
    // For a SELL, match with buy orders priced at or above the order's price.
    matchCount = process_orders(order, orderbook.buyOrders, std::greater<>());
    if (order.quantity > 0)
      orderbook.sellOrders[order.price].insert({order.id, order});
  }
  return matchCount;
}

// Templated helper to cancel an order within a given orders map.
template <typename OrderMap>
bool modify_order_in_map(OrderMap &ordersMap, IdType order_id,
                         QuantityType new_quantity) {
  for (auto it = ordersMap.begin(); it != ordersMap.end();) {
    auto &orderIdMap = it->second;
    auto theOrder = orderIdMap.find(order_id);
    if (theOrder != orderIdMap.end()) {
      if (new_quantity == 0)
        orderIdMap.erase(theOrder);
      else
        theOrder->second.quantity = new_quantity;
      return true;
    }
    
    if (orderIdMap.empty())
      it = ordersMap.erase(it);
    else
      ++it;
  }
  return false;
}

void modify_order_by_id(Orderbook &orderbook, IdType order_id,
                        QuantityType new_quantity) {
  if (modify_order_in_map(orderbook.buyOrders, order_id, new_quantity))
    return;
  if (modify_order_in_map(orderbook.sellOrders, order_id, new_quantity))
    return;
}

template <typename OrderMap>
std::optional<Order> lookup_order_in_map(OrderMap &ordersMap, IdType order_id) {
  for (const auto &[price, orderIdMap] : ordersMap) {
    auto it = orderIdMap.find(order_id);
    if (it != orderIdMap.end()) {
      return it->second;
    }
  }
  return std::nullopt;
}

uint32_t get_volume_at_level(Orderbook &orderbook, Side side,
                             PriceType quantity) {
  uint32_t total = 0;
  if (side == Side::BUY) {
    auto buy_orders = orderbook.buyOrders.find(quantity);
    if (buy_orders == orderbook.buyOrders.end()) {
      return 0;
    }
    for (const auto &order : buy_orders->second) {
      total += order.second.quantity;
    }
  } else if (side == Side::SELL) {
    auto sell_orders = orderbook.sellOrders.find(quantity);
    if (sell_orders == orderbook.sellOrders.end()) {
      return 0;
    }
    for (const auto &order : sell_orders->second) {
      total += order.second.quantity;
    }
  }
  return total;
}

// Functions below here don't need to be performant. Just make sure they're
// correct
Order lookup_order_by_id(Orderbook &orderbook, IdType order_id) {
  auto order1 = lookup_order_in_map(orderbook.buyOrders, order_id);
  auto order2 = lookup_order_in_map(orderbook.sellOrders, order_id);
  if (order1.has_value())
    return *order1;
  if (order2.has_value())
    return *order2;
  throw std::runtime_error("Order not found");
}

bool order_exists(Orderbook &orderbook, IdType order_id) {
  auto order1 = lookup_order_in_map(orderbook.buyOrders, order_id);
  auto order2 = lookup_order_in_map(orderbook.sellOrders, order_id);
  return (order1.has_value() || order2.has_value());
}

Orderbook *create_orderbook() { return new Orderbook; }
