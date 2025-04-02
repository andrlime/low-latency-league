// some shared types
#pragma once
#include <list>

enum class Side : uint8_t { BUY, SELL };

using IdType = uint32_t;
using PriceType = uint16_t;
using QuantityType = uint16_t;

struct Order {
  IdType id; // Unique
  PriceType price;
  QuantityType quantity;
  Side side;
};

using OrderListType = std::list<Order>;


#define DODEBUG 0
#if DODEBUG

#include <iostream>
#define DEBUG(x) std::cerr << x << std::endl;

#else

#define DEBUG(x) 

#endif

