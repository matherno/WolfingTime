#include <mathernogl/MathernoGL.h>
#include "WolfRenderer.h"
#include "WolfMap.h"
#include "MapImporter.h"

#define WINDOW_WIDTH  1200
#define WINDOW_HEIGHT 700
#define SCREENTEX_WIDTH  WINDOW_WIDTH
#define SCREENTEX_HEIGHT WINDOW_HEIGHT
#define MINIMAP_WIDTH   400
#define MINIMAP_HEIGHT  250

long getTimeMS()
  {
  using namespace std::chrono;
  milliseconds ms = duration_cast<milliseconds>(
    system_clock::now().time_since_epoch()
  );
  return ms.count();
  }

void displayHelp()
  {
  std::cout << "WolfingTime [-d,-m] Map.wolf" << std::endl;
  std::cout << "Use -d to enable delay load mode" << std::endl;
  std::cout << "Use -m to enable mini map" << std::endl;
  std::cout << "WASD moves player, mouse movement for rotation" << std::endl;
  std::cout << "Hold left shift to move faster" << std::endl;
  std::cout << "ESC to exit" << std::endl;
  }

int main(int argc, char* args[])
  {
  bool delayMode = false;
  bool miniMap = false;
  for (int idx = 1; idx < argc; ++idx)
    {
    if (std::string(args[idx]) == "-d")
      delayMode = true;

    if (std::string(args[idx]) == "-m")
      miniMap = true;

    if (std::string(args[idx]) == "-h" || std::string(args[idx]) == "-help")
      {
      displayHelp();
      return 0;
      }
    }

  if (argc < 2)
    {
    displayHelp();
    return 0;
    }

  // load window and initialise renderer
  std::unique_ptr<mathernogl::Window> window(mathernogl::initGL("WolfingTime", WINDOW_WIDTH, WINDOW_HEIGHT));
  window->setClearColour(1, 0, 0.5);
  WolfRenderer renderer;
  renderer.initialise(SCREENTEX_WIDTH, SCREENTEX_HEIGHT, delayMode);
  if (miniMap)
    renderer.initialiseMiniMap(MINIMAP_WIDTH, MINIMAP_HEIGHT);

  //  load map
  WolfMapPtr map;
  std::string mapPath = args[argc - 1];
  try
    {
    map = MapImporter::importMap(mapPath);
    }
  catch (std::runtime_error& err)
    {
    mathernogl::logError("Failed to load map: " + mapPath);
    return 0;
    }

  mathernogl::InputHandler inputHandler;
  inputHandler.init(window->getGLFWWindow());
  const float playerSpeed = 0.15;
  const float playerRotSpeed = 0.15;
  const float shiftSpeedMod = 2;
  Vector2D mousePos = inputHandler.getMousePos();
  inputHandler.setCursorMode(mathernogl::CursorMode::CURSOR_DISABLED);
  bool firstFrame = true;

  while(window->isOpen())
    {
    long startTime = getTimeMS();
    inputHandler.checkHeldButtons();
    if (inputHandler.isKeyAction(GLFW_KEY_ESCAPE, mathernogl::INPUT_HELD))
        break;

    //  do player rotation
    float playerRot = map->getPlayerRot();
    Vector2D newMousePos = inputHandler.getMousePos();
    if (!firstFrame)
      {
      playerRot += playerRotSpeed * (newMousePos.x - mousePos.x);
      map->setPlayerRot(playerRot);
      }
    mousePos = newMousePos;

    //  retrieve player movement
    Vector2D lookDirection = WolfRenderer::calcDirection(playerRot);
    Vector2D rightDirection = WolfRenderer::calcDirection(playerRot + 90);
    Vector2D movement(0, 0);
    if(inputHandler.isKeyAction(GLFW_KEY_W, mathernogl::INPUT_HELD))
      movement.y += 1;
    if(inputHandler.isKeyAction(GLFW_KEY_S, mathernogl::INPUT_HELD))
      movement.y -= 1;
    if(inputHandler.isKeyAction(GLFW_KEY_D, mathernogl::INPUT_HELD))
      movement.x += 1;
    if(inputHandler.isKeyAction(GLFW_KEY_A, mathernogl::INPUT_HELD))
      movement.x -= 1;
    Vector2D playerPos = map->getPlayerPos();

    //  do player movement if valid
    float speed = playerSpeed;
    if (inputHandler.isKeyAction(GLFW_KEY_LEFT_SHIFT, mathernogl::INPUT_HELD))
      speed *= shiftSpeedMod;
    Vector2D newPlayerPos = playerPos + (rightDirection * movement.x + lookDirection * movement.y) * speed;
    if (!map->isObstacle((uint)newPlayerPos.x, (uint)newPlayerPos.y))
      map->setPlayerPos(newPlayerPos);

    //  render and update
    window->clear();
    renderer.render(map.get());
    window->update();
    inputHandler.clearEvents();
    firstFrame = false;
    while (getTimeMS() - startTime < 33) {}
    }

  inputHandler.cleanUp();
  renderer.cleanUp();

  return 0;
  }