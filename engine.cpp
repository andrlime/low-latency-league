#include "engine.hpp"
#include <functional>
#include <optional>
#include <stdexcept>


// Templated helper to process matching orders.
// The Condition predicate takes the price level and the incoming order price
// and returns whether the level qualifies.
// TODO: need to update total volume
template <typename OrderMap, typename Condition>
uint32_t process_orders(Order &order, OrderMap &ordersMap, Condition cond) {
  uint32_t matchCount = 0;
  auto it = ordersMap.begin();

  DEBUG("Processing orders: " << order.price << " " << order.quantity);
  DEBUG("start_it: " << it->first);

  // need to match levels 
  auto end_it = ordersMap.end();
  while (it != end_it && order.quantity > 0 &&
         (it->first == order.price || cond(it->first, order.price))) {
    DEBUG("Processing level: " << it->first);
    auto &ordersAtPrice = it->second;
    for (auto orderIt = ordersAtPrice.begin();
         orderIt != ordersAtPrice.end() && order.quantity > 0;) {
      QuantityType trade = std::min(order.quantity, orderIt->quantity);
      order.quantity -= trade;
      orderIt->quantity -= trade;
      ordersAtPrice.total_volume -= trade;
      ++matchCount;
      if (orderIt->quantity == 0) { [[unlikely]]
        orderIt = ordersAtPrice.erase(orderIt);
      } else { [[likely]]
        ++orderIt;
      }

      if (ordersAtPrice.empty()) { [[unlikely]]
        it = ordersMap.erase(it);
      } else {
        ++it;
      }
    }
  }

  DEBUG("returning matchCount: " << matchCount);
  return matchCount;
}

uint32_t match_order(Orderbook &orderbook, const Order &incoming) {
  uint32_t matchCount = 0;
  Order order = incoming; // Create a copy to modify the quantity
  
  DEBUG("Matching order: " << order.id << " " << order.price << " "
                           << order.quantity << " " << (int)order.side);

  if (order.side == Side::BUY) {
    matchCount = process_orders(order, orderbook.sellOrders, std::less<>());
  } else { // Side::SELL
    matchCount = process_orders(order, orderbook.buyOrders, std::greater<>());
  }
  if (order.quantity > 0) { [[likely]]
    orderbook.push_back(order);
  }
  
  return matchCount;
}

// Templated helper to cancel an order within a given orders map.
// TODO: make this function update total volume
template <typename OrderMap>
bool modify_order_in_map(OrderMap &ordersMap, IdType order_id,
                         QuantityType new_quantity) {
  for (auto it = ordersMap.begin(); it != ordersMap.end();) {
    auto &orderList = it->second;
    for (auto orderIt = orderList.begin(); orderIt != orderList.end();) {
      if (orderIt->id == order_id) {
        if (new_quantity == 0) {
          orderIt = orderList.erase(orderIt);
        } else {
          orderList.total_volume -= orderIt->quantity;
          orderIt->quantity = new_quantity;
          orderList.total_volume += orderIt->quantity;
          return true;
        }
      } else {
        ++orderIt;
      }
    }
    if (orderList.empty())
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
  for (const auto &pair : ordersMap) {
    const auto &orderList = pair.second;
    for (const auto &order : orderList.get_orders()) {
      if (order.id == order_id) {
        return order;
      }
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
    return buy_orders->second.total_quantity();
  } else if (side == Side::SELL) {
    auto sell_orders = orderbook.sellOrders.find(quantity);
    if (sell_orders == orderbook.sellOrders.end()) {
      return 0;
    }
    return sell_orders->second.total_quantity();
  }
  return total;
}

// Functions below here don't need to be performant. Just make sure they're
// correct
Order lookup_order_by_id(Orderbook &orderbook, IdType order_id) {
  auto order1 = lookup_order_in_map(orderbook.buyOrders, order_id);
  if (order1.has_value())
    return *order1;
  
  auto order2 = lookup_order_in_map(orderbook.sellOrders, order_id);
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
