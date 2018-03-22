//
// Created by matt on 14/03/18.
//

#include <set>
#include "MapImporter.h"
#include "WolfRenderer.h"

#define UNDEFINED 99999
#define FIRST_LINE_VERIFICATION "wolfingtime_map_def"

bool getVariablesValue(const std::string& line, const std::string& variableName, std::string& value)
  {
  unsigned long lineIdx = line.find(variableName);
  if (lineIdx != std::string::npos)
    {
    lineIdx = line.find(' ');
    if (lineIdx != std::string::npos)
      {
      value = line.substr(lineIdx + 1);
      return true;
      }
    }
  return false;
  }

//  wall value => [cell num] [texture file path]
bool parseWallValue(const std::string& wallValue, uint& cellID, std::string& filePath)
  {
  unsigned long idx = wallValue.find(' ');
  if (idx != std::string::npos)
    {
    cellID = (uint)atoi(wallValue.substr(0, idx).c_str());
    if (cellID == 0)
      return false;
    cellID += WALL_IDS_START - 1;
    filePath = wallValue.substr(idx + 1, wallValue.size()-idx-2);
    return true;
    }
  return false;
  }

void throwError(std::string msg)
  {
  mathernogl::logError(msg);
  throw std::runtime_error(msg);
  }

WolfMapPtr MapImporter::importMap(const std::string& filePath)
  {
  uint width = UNDEFINED;
  uint height = UNDEFINED;
  uint playerPosX = 1;
  uint playerPosY = 1;
  float playerRot = 0;
  Vector3D ceilingColour = Vector3D(0.4);
  Vector3D floorColour = Vector3D(0.5, 0.3, 0.2);
  std::map<uint, std::string> cellTypes;

  //  verify it's a WolfingTime map file
  std::ifstream infile(filePath);
  std::string line;
  if (std::getline(infile, line))
    {
    if (line.compare(0, line.size()-1, FIRST_LINE_VERIFICATION) != 0)
      throwError("Not a valid map file!");
    }
  else
    throwError("File is empty!");

  //  read map attribute variables
  while (std::getline(infile, line))
    {
    std::string value = "";
    if (getVariablesValue(line, "width", value))
      width = (uint)atoi(value.c_str());
    else if (getVariablesValue(line, "height", value))
      height = (uint)atoi(value.c_str());

    else if (getVariablesValue(line, "playerposx", value))
      playerPosX = (uint)atoi(value.c_str());
    else if (getVariablesValue(line, "playerposy", value))
      playerPosY = (uint)atoi(value.c_str());
    else if (getVariablesValue(line, "playerrot", value))
      playerRot = (uint)atoi(value.c_str());

    else if (getVariablesValue(line, "ceilingr", value))
      ceilingColour.x = atof(value.c_str());
    else if (getVariablesValue(line, "ceilingg", value))
      ceilingColour.y = atof(value.c_str());
    else if (getVariablesValue(line, "ceilingb", value))
      ceilingColour.z = atof(value.c_str());

    else if (getVariablesValue(line, "floorr", value))
      floorColour.x = atof(value.c_str());
    else if (getVariablesValue(line, "floorg", value))
      floorColour.y = atof(value.c_str());
    else if (getVariablesValue(line, "floorb", value))
      floorColour.z = atof(value.c_str());

    else if (getVariablesValue(line, "wall", value))
      {
      uint cellID;
      std::string filePath;
      if (parseWallValue(value, cellID, filePath))
        cellTypes[cellID] = filePath;
      }

    else if (line.compare(0, line.size()-1, "map") == 0)
      break;
    }

  if (width == UNDEFINED || height == UNDEFINED)
    throwError("Map width or height undefined in imported map!");
  if (width == 0 || height == 0 || width > 999 || height > 999)
    throwError("Map width or height invalid in imported map!");

  //  create map object
  WolfMap* wolfMap = new WolfMap(width, height);
  if (!wolfMap->isValidCell(playerPosX, playerPosY) || wolfMap->isObstacle(playerPosX, playerPosY))
    throwError("Map player position is invalid within the given map grid!");
  wolfMap->setPlayerPos(Vector2D(playerPosX + 0.5f, playerPosY + 0.5f));
  wolfMap->setPlayerRot(playerRot);
  wolfMap->setCeilingColour(ceilingColour / 255);
  wolfMap->setFloorColour(floorColour / 255);

  //  load cell types and textures into map object
  try
    {
    for (auto pair : cellTypes)
      {
      TexturePtr texture(mathernogl::createTextureFromFile(pair.second, false));
//      preFilterTexture(texture);
      wolfMap->setCellIDTexture((CellID)pair.first, texture);
      }
    }
  catch (std::runtime_error& err)
    {
    throwError(err.what());
    }

  //  read map layout into map object
  uint row = height - 1;
  while (row >= 0 && std::getline(infile, line))
    {
    uint numChars = std::min(width, (uint)line.size()-1);
    for (int col = 0; col < numChars; ++col)
      {
      uint cellID = (uint)(line[col] - '0');
      if (cellID == 0)
        cellID = CELL_EMPTY;
      else
        cellID += WALL_IDS_START - 1;
      wolfMap->setCell((CellID)col, row, (CellID)cellID);
      }
    --row;
    }

  return WolfMapPtr(wolfMap);
  }
