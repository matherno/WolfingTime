//
// Created by matt on 10/03/18.
//

#include <mathernogl/MathernoGL.h>
#include <cfloat>
#include "WolfRenderer.h"

#define MINIMAP_CELL_SIZE 15
#define VIEW_COL   Vector3D(0, 0.5, 0)
#define EMPTY_COL  Vector3D(1, 1, 1)
#define WALL_COL   Vector3D(0, 0, 0)

void WolfRenderer::initialise(uint width, uint height, bool delayRenderMode)
  {
  screenTexture.reset(mathernogl::createEmptyTexture(width, height, mathernogl::NEAREST));
  screenRenderer.init();
  screenRenderer.setTexture(screenTexture);
  byteWriter.setTexture(screenTexture);

  vertFOV = horizFOV * ((float)screenTexture->height / screenTexture->width);
  this->delayRenderMode = delayRenderMode;
  }

void WolfRenderer::initialiseMiniMap(uint width, uint height)
  {
  miniMapTexture.reset(mathernogl::createEmptyTexture(width, height, mathernogl::NEAREST));
  }

void WolfRenderer::render(const WolfMap* map)
  {
  if (miniMapTexture)
    drawMiniMapCells(map);
  drawFloorAndCeiling(map);
  drawMapCells(map);
  if (miniMapTexture)
    {
    mathernogl::updateTexture(miniMapTexture.get());
    byteWriter.writeTexture(0, 0, miniMapTexture);
    }
  byteWriter.updateTexture();
  screenRenderer.render();
  }

void WolfRenderer::cleanUp()
  {

  }

void WolfRenderer::drawFloorAndCeiling(const WolfMap* map)
  {
  const uint halfHeight = (uint)(screenTexture->height * 0.5);
  byteWriter.fillRect(0, 0, screenTexture->width, halfHeight, map->getFloorColour());
  byteWriter.fillRect(0, halfHeight, screenTexture->width, screenTexture->height, map->getCeilingColour());
  }

void WolfRenderer::drawMapCells(const WolfMap* map)
  {
  uint screenWidth = screenTexture->width;
  if (delayRenderMode)
    screenWidth = std::min(screenWidth, colDelayCount);
  colDelayCount += colDelayInc;
  colDelayCount = colDelayCount % screenTexture->width;

  for (uint screenCol = 0; screenCol < screenWidth; ++screenCol)
    {
    const Vector2D playerPos = map->getPlayerPos();
    const float playerRot = map->getPlayerRot();
    float rayRot = screenCol * horizFOV / (screenTexture->width - 1);
    rayRot -= (horizFOV * 0.5f);
    rayRot += playerRot;
    CellID hitCellID = CELL_EMPTY;
    Vector2D hitCellPos;
    FaceDirection hitFaceDirection;

    hitTestCell(playerPos, rayRot, map, &hitCellID, &hitCellPos, &hitFaceDirection);
    float hitOrthoDistance = calcOrthogonalDistance(playerPos, playerRot, hitCellPos);
    if (hitCellID >= WALL_IDS_START)
      {
      drawWall(screenCol, map->getWallTexture(hitCellID), hitOrthoDistance, hitCellPos, hitFaceDirection);

      if (miniMapTexture)
        {
        ByteWriter writer;
        writer.setTexture(miniMapTexture);
        const uint miniMapWidth = miniMapTexture->width;
        const uint miniMapHeight = miniMapTexture->height;
        const uint miniMapPlayerX = (uint)((float)miniMapWidth * 0.5);
        const uint miniMapPlayerY = (uint)((float)miniMapHeight * 0.5);

        float miniMapCol = hitCellPos.x + (float)miniMapPlayerX / MINIMAP_CELL_SIZE;
        float miniMapRow = hitCellPos.y + (float)miniMapPlayerY / MINIMAP_CELL_SIZE;
        miniMapCol -= map->getPlayerPos().x;
        miniMapRow -= map->getPlayerPos().y;
        miniMapCol *= MINIMAP_CELL_SIZE;
        miniMapRow *= MINIMAP_CELL_SIZE;
        writer.writeLine(miniMapPlayerX, miniMapPlayerY, miniMapCol, miniMapRow, VIEW_COL);
        }

      }
    }
  }

float getTextureCoordX(const Vector2D& wallHitPos, FaceDirection wallFaceDir)
  {
  float stub;
  if (wallFaceDir == directionPosX || wallFaceDir == directionNegX)
    {
    float texCoordX = modff(wallHitPos.y, &stub);
    return wallFaceDir == directionPosX ? texCoordX : 1.0f - texCoordX;
    }
  else
    {
    float texCoordX = modff(wallHitPos.x, &stub);
    return wallFaceDir == directionNegY ? texCoordX : 1.0f - texCoordX;
    }
  }

void WolfRenderer::drawWall(uint screenCol, TexturePtr texture, float orthoDistance, const Vector2D& wallHitPos, FaceDirection wallFaceDir)
  {
  const double wallPixelHeight = mathernogl::radToDeg(2 * atanf(0.5f / orthoDistance)) * screenTexture->height / vertFOV;
  const double wallStartY = (screenTexture->height - wallPixelHeight) * 0.5;
  const uint startPixelY = (uint)std::max(0.0, wallStartY);
  const uint endPixelY = (uint)std::min((double)(screenTexture->height - 1), wallStartY + wallPixelHeight);

  Vector2D textureCoord;
  textureCoord.x = getTextureCoordX(wallHitPos, wallFaceDir);
  for (uint pixelY = startPixelY; pixelY <= endPixelY; ++pixelY)
    {
    textureCoord.y = (float) (pixelY - wallStartY) / (float) wallPixelHeight;
    byteWriter.writePixel(screenCol, pixelY, ByteWriter::sampleTexture(texture, textureCoord, true));
    }
  }

Vector2D WolfRenderer::calcDirection(float rot)
  {
  Vector2D viewDir;
  const float theta = (float)mathernogl::degToRad(rot);
  viewDir.x = sinf(theta);
  viewDir.y = cosf(theta);
  viewDir.makeUniform();
  return viewDir;
  }

#define CLAMP(x, y) mathernogl::clampi((int)x, 0, y)

void getCellContainingPoint(const Vector2D& point, const WolfMap* map, int* cellX, int* cellY)
  {
  *cellX = CLAMP(point.x, map->getWidth()-1);
  *cellY = CLAMP(point.y, map->getHeight()-1);
  }

bool WolfRenderer::hitTestCell(const Vector2D& rayPos, float rayRot, const WolfMap* map, CellID* cellID, Vector2D* hitPoint, FaceDirection* hitFaceDirection)
  {
  const Vector2D rayDir = calcDirection(rayRot);
  const Vector2D gridSize = Vector2D(map->getWidth(), map->getHeight());

  //  find which cell to start in (assume ray origin is in grid)
  int cellX, cellY;
  getCellContainingPoint(rayPos, map, &cellX, &cellY);

  //  calculate the cell t offset and next values, and cell increments
  float tXOffset = 0, tYOffset = 0;
  float tXNext = FLT_MAX, tYNext = FLT_MAX;
  int cellXInc = 0, cellYInc = 0;

  if (rayDir.x != 0)
    {
    float tXLB = (0 - rayPos.x) / rayDir.x;
    float tXUB = (gridSize.x - rayPos.x) / rayDir.x;
    cellXInc = rayDir.x >= 0 ? 1 : -1;
    tXOffset = (float)fabs(tXLB - tXUB) / map->getWidth();
    if (tXLB < tXUB)
      tXNext = tXLB + tXOffset * (cellX + 1);
    else
      tXNext = tXUB + tXOffset * (map->getWidth() - cellX);
    }

  if (rayDir.y != 0)
    {
    float tYLB = (0 - rayPos.y) / rayDir.y;
    float tYUB = (gridSize.y - rayPos.y) / rayDir.y;
    cellYInc = rayDir.y >= 0 ? 1 : -1;
    tYOffset = (float) fabs(tYLB - tYUB) / map->getHeight();
    if (tYLB < tYUB)
      tYNext = tYLB + tYOffset * (cellY + 1);
    else
      tYNext = tYUB + tYOffset * (map->getHeight() - cellY);
    }

  //  traverse through the cells that the ray goes through, and hit test the objects in them
  float thisTValue = std::max(tXNext - tXOffset, tYNext - tYOffset);
  FaceDirection direction = FaceDirection(-1);
  while (true) {

    if (direction != -1)    // skip cell that ray starts in for now
      {
      //  check that the hit point is within the cell
      CellID thisCellID = map->getCell(cellX, cellY);
      if (thisCellID != CELL_EMPTY)
        {
        *cellID = thisCellID;
        *hitPoint = rayPos + rayDir * thisTValue;
        *hitFaceDirection = direction;
        return true;
        }
      }
    thisTValue = std::min(tXNext, tYNext);

    //  work out which cell is next
    if (tXNext < tYNext)
      {
      cellX += cellXInc;
      tXNext += tXOffset;
      direction = cellXInc > 0 ? directionNegX : directionPosX;
      }
    else
      {
      cellY += cellYInc;
      tYNext += tYOffset;
      direction = cellYInc > 0 ? directionNegY : directionPosY;
      }

    //  abort if we've left the grid
    if (cellX < 0 || cellY < 0 || cellX >= map->getWidth() || cellY >= map->getHeight())
      return false;

    }
  }

float WolfRenderer::calcOrthogonalDistance(const Vector2D& playerPos, float playerRot, const Vector2D& point)
  {
  const Vector2D viewDir = calcDirection(playerRot);
  return (float)mathernogl::dotProduct(point - playerPos, viewDir);
  }

void WolfRenderer::drawMiniMapCells(const WolfMap* map)
  {
  const uint miniMapWidth = miniMapTexture->width;
  const uint miniMapHeight = miniMapTexture->height;
  const uint miniMapPlayerX = (uint)((float)miniMapWidth * 0.5);
  const uint miniMapPlayerY = (uint)((float)miniMapHeight * 0.5);

  ByteWriter writer;
  writer.setTexture(miniMapTexture);

  for (uint row = 0; row < miniMapHeight; ++row)
    {
    for (uint col = 0; col < miniMapWidth; ++col)
      {
      float mapCol = (float)col / MINIMAP_CELL_SIZE;
      float mapRow = (float)row / MINIMAP_CELL_SIZE;
      mapCol += map->getPlayerPos().x;
      mapRow += map->getPlayerPos().y;
      mapCol -= (float)miniMapPlayerX / MINIMAP_CELL_SIZE;
      mapRow -= (float)miniMapPlayerY / MINIMAP_CELL_SIZE;

      bool isWall;
      if (map->isValidCell(mapCol, mapRow))
        isWall = map->isObstacle(mapCol, mapRow);
      else
        isWall = true;

      writer.writePixel(col, row, isWall ? WALL_COL : EMPTY_COL);
      }
    }
  }

