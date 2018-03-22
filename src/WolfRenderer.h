#pragma once

#include <mathernogl/Types.h>
#include <mathernogl/rendering/ScreenTextureRenderer.h>
#include <mathernogl/systemio/Window.h>
#include "ByteWriter.h"
#include "WolfMap.h"

/*
*   
*/

typedef mathernogl::Vector2D Vector2D;

class WolfRenderer
  {
private:
  TexturePtr screenTexture;
  mathernogl::ScreenTextureRenderer screenRenderer = mathernogl::ScreenTextureRenderer(true);
  ByteWriter byteWriter;
  TexturePtr miniMapTexture;

  float horizFOV = 80;    // angle degrees
  float vertFOV = 20;

  // when delayRenderMode == true, each render call increments colDelayCount by colDelayInc, colDelayCount determining how many pixel columns of the screen are allowed to be drawn
  bool delayRenderMode = false;
  uint colDelayInc = 4;
  uint colDelayCount = 0;

public:
  void initialise(uint width, uint height, bool delayRenderMode = false);
  void initialiseMiniMap(uint width, uint height);
  void render(const WolfMap* map);
  void cleanUp();

  /*
   *  Hit tests the cells in the given map
   *  Returns true if cell was hit
   *  cellID => the id of the cell that was hit
   *  hitPoint => the hit point of the cell
   */
  static bool hitTestCell(const Vector2D& rayPos, float rayRot, const WolfMap* map, CellID* cellID, Vector2D* hitPoint, FaceDirection* hitFaceDirection);

  static float calcOrthogonalDistance(const Vector2D& playerPos, float playerRot, const Vector2D& point);
  static Vector2D calcDirection(float rotation);

protected:
  void drawFloorAndCeiling(const WolfMap* map);
  void drawMapCells(const WolfMap* map);
  void drawWall(uint screenCol, TexturePtr texture, float orthoDistance, const Vector2D& wallHitPos, FaceDirection wallFaceDir);
  void drawMiniMapCells(const WolfMap* map);
  };
