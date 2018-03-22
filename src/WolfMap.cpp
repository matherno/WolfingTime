//
// Created by matt on 10/03/18.
//

#include <mathernogl/Assert.h>
#include "WolfMap.h"

WolfMap::WolfMap(uint width, uint height) : width(width), height(height)
  {
  for (int cell = 0; cell < width * height; ++cell)
    cells.push_back(CELL_EMPTY);
  }

void WolfMap::setCell(uint col, uint row, CellID id)
  {
  if (isValidCell(col, row))
    cells[col + row * width] = id;
  }

CellID WolfMap::getCell(uint col, uint row) const
  {
  ASSERT(isValidCell(col, row), "Out of bounds: col: " + std::to_string(col) + ", row: " + std::to_string(row));
  return cells.at(col + row * width);
  }

bool WolfMap::isValidCell(uint col, uint row) const
  {
  return col >= 0 && col < width && row >= 0 && row < height;
  }

std::string WolfMap::createVisualString() const
  {
  std::string map = "";

  for (int row = height - 1; row >= 0; --row)
    {
    for (int col = 0; col < width; ++col)
      {
      CellID cellID = getCell(col, row);
      char cellChar = ' ';
      if (cellID == CELL_DOOR)
        cellChar = 'D';
      else if (cellID >= WALL_IDS_START)
        cellChar = '#';
      map += cellChar;
      }
    map += '\n';
    }

  return map;
  }

bool WolfMap::isObstacle(uint col, uint row) const
  {
  return isValidCell(col, row) && getCell(col, row) >= WALL_IDS_START;
  }

TexturePtr WolfMap::getWallTexture(CellID cellID) const
  {
  if (cellTextures.count(cellID) > 0)
    return cellTextures.at(cellID);
  return nullptr;
  }

void WolfMap::setCellIDTexture(CellID id, TexturePtr colour)
  {
  cellTextures[id] = colour;
  }
