#include "blocks.h"

const int blocks[256][6] = {
  // w => (left, right, top, bottom, front, back) tiles
  {0, 0, 0, 0, 0, 0}, // 0 - empty
  {4, 4, 8, 0, 8, 4}, // 1 - grass
  {1, 1, 1, 1, 1, 1}, // 2 - stone
  {0, 0, 0, 0, 0, 0}, // 3 - dirt
  {2, 2, 2, 2, 2, 2}, // 4 - bedrock
  {3, 3, 3, 3, 3, 3}, // 5 - sand
};
