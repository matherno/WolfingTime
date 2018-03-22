//
// Created by matt on 10/03/18.
//

#include <mathernogl/MathernoGL.h>
#include "ByteWriter.h"

void ByteWriter::updateTexture()
  {
  if (texture)
    mathernogl::updateTexture(texture.get());
  }

void ByteWriter::writePixel(uint col, uint row, const Vector3D& colour)
  {
  if (col < texture->width && row < texture->height)
    {
    uint redIndex = col * texture->bytesPerPixel + row * texture->width * texture->bytesPerPixel;
    texture->bytes.get()[redIndex]      = (unsigned char) (colour.x * 255);
    texture->bytes.get()[redIndex + 1]  = (unsigned char) (colour.y * 255);
    texture->bytes.get()[redIndex + 2]  = (unsigned char) (colour.z * 255);
    }
  }

void ByteWriter::fillRect(uint bottomLeftCol, uint bottomLeftRow, uint width, uint height, const Vector3D& colour)
  {
  for (uint row = 0; row < height; ++row)
    {
    for (uint col = 0; col < width; ++col)
      {
      writePixel(bottomLeftCol + col, bottomLeftRow + row, colour);
      }
    }
  }

void ByteWriter::writeTexture(uint bottomLeftCol, uint bottomLeftRow, TexturePtr texture)
  {
  for (uint row = 0; row < texture->height; ++row)
    {
    for (uint col = 0; col < texture->width; ++col)
      {
      writePixel(bottomLeftCol + col, bottomLeftRow + row, sampleTexture(texture, col, row, false));
      }
    }
  }

Vector3D ByteWriter::sampleTexture(TexturePtr texture, const mathernogl::Vector2D& textureCoord, bool bgr)
  {
  int col = (int)roundf(textureCoord.x * texture->width);
  int row = (int)roundf(textureCoord.y * texture->height);
  col = mathernogl::clampi(col, 0, texture->width-1);
  row = mathernogl::clampi(row, 0, texture->height-1);
  return sampleTexture(texture, col, row, bgr);
  }

Vector3D ByteWriter::sampleTexture(TexturePtr texture, uint col, uint row, bool bgr)
  {
  Vector3D colour;
  uint firstIndex = col * texture->bytesPerPixel + row * texture->width * texture->bytesPerPixel;
  colour.x = texture->bytes.get()[firstIndex];
  colour.y = texture->bytes.get()[firstIndex + 1];
  colour.z = texture->bytes.get()[firstIndex + 2];
  colour /= 255;
  if (bgr)
    colour.set(colour.z, colour.y, colour.x);
  return colour;
  }

void ByteWriter::writeLine(uint startCol, uint startRow, uint endCol, uint endRow, const Vector3D& colour)
  {
  int lineWidth = (int)endCol - (int)startCol;
  int lineHeight = (int)endRow - (int)startRow;

  float deltaX = fabs((float)lineWidth / (float)lineHeight);
  float deltaY = fabs((float)lineHeight / (float)lineWidth);

  int numPixels;
  if (deltaX > deltaY)
    {
    numPixels = abs(lineWidth);
    deltaX = 1.0;
    }
  else
    {
    numPixels = abs(lineHeight);
    deltaY = 1.0;
    }

  deltaX *= lineWidth >= 0 ? 1 : -1;
  deltaY *= lineHeight >= 0 ? 1 : -1;

  float col = startCol + 0.5f * deltaX;
  float row = startRow + 0.5f * deltaY;
  for (int pixelNum = 0; pixelNum <= numPixels; ++pixelNum)
    {
    writePixel((uint)col, (uint)row, colour);
    col += deltaX;
    row += deltaY;
    }
  }
