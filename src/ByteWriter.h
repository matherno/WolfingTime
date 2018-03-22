#pragma once

#include <mathernogl/texture/Texture.h>

/*
*   Going with the convention of <0,0> being bottom left, so <width,height> is top right
*/

typedef mathernogl::Vector3D Vector3D;
typedef mathernogl::Texture Texture;
typedef std::shared_ptr<Texture> TexturePtr;

class ByteWriter
  {
private:
  TexturePtr texture;

public:
  void setTexture(TexturePtr texture) { this->texture = texture; }
  void updateTexture();

  void writePixel(uint col, uint row, const Vector3D& colour);
  void fillRect(uint bottomLeftCol, uint bottomLeftRow, uint width, uint height, const Vector3D& colour);
  void writeTexture(uint bottomLeftCol, uint bottomLeftRow, TexturePtr texture);
  void writeLine(uint startCol, uint startRow, uint endCol, uint endRow, const Vector3D& colour);

  //  if not bgr format, then it's rgb
  static Vector3D sampleTexture(TexturePtr texture, const mathernogl::Vector2D& textureCoord, bool bgr);
  static Vector3D sampleTexture(TexturePtr texture, uint col, uint row, bool bgr);
  };
