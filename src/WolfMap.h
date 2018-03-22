#pragma once

#include <mathernogl/Types.h>
#include <vector>
#include <memory>
#include <mathernogl/MathernoGL.h>
#include "ByteWriter.h"

/*
*   
*/

// cell identifiers...
#define CELL_EMPTY  0
#define CELL_DOOR   1   // not in use yet

//  any id after and including WALL_IDS_START are walls
#define WALL_IDS_START  10

typedef unsigned char CellID;

enum FaceDirection
  {
  directionPosX,
  directionNegX,
  directionPosY,
  directionNegY,
  };

class WolfMap
  {
private:
  const uint width;
  const uint height;
  std::vector<CellID> cells;
  std::map<CellID, TexturePtr> cellTextures;
  mathernogl::Vector2D playerPos;
  float playerRot = 0;
  Vector3D floorColour = Vector3D(0.3, 0.2, 0.1);
  Vector3D ceilingColour = Vector3D(0.2, 0.2, 0.2);

public:
  WolfMap(uint width, uint height);
  void setCell(uint col, uint row, CellID id);
  CellID getCell(uint col, uint row) const;
  void setCellIDTexture(CellID id, TexturePtr colour);
  bool isValidCell(uint col, uint row) const;
  std::string createVisualString() const;
  uint getWidth() const { return width; }
  uint getHeight() const { return height; }
  bool isObstacle(uint col, uint row) const;
  TexturePtr getWallTexture(CellID cellID) const;
  mathernogl::Vector2D getPlayerPos() const { return playerPos; }
  float getPlayerRot() const { return playerRot; }
  void setPlayerPos(const mathernogl::Vector2D& pos) { playerPos = pos; }
  void setPlayerRot(float rot) { playerRot = rot; }
  void setFloorColour(const Vector3D& col){ floorColour = col; }
  void setCeilingColour(const Vector3D& col){ ceilingColour = col; }
  Vector3D getFloorColour() const { return floorColour; }
  Vector3D getCeilingColour() const { return ceilingColour; }
  };

typedef std::shared_ptr<WolfMap> WolfMapPtr;