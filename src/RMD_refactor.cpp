#include <Corrade/Containers/GrowableArray.h>
#include <Corrade/Containers/Optional.h>
#include <Corrade/Containers/Pointer.h>
#include <Corrade/Utility/Arguments.h>

#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/DebugTools/FrameProfiler.h>
#include <Magnum/DebugTools/ColorMap.h>

#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/GL/PixelFormat.h>

#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>

#include <Magnum/Primitives/Icosphere.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/Primitives/Grid.h>

#include <Magnum/Shaders/FlatGL.h>
#include <Magnum/Shaders/VertexColorGL.h>
#include <Magnum/Shaders/MeshVisualizerGL.h>
#include <Magnum/Shaders/PhongGL.h>

#include <Magnum/Trade/MeshData.h>
#include <Magnum/Trade/Data.h>

#include <Magnum/MeshTools/Compile.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/FlipNormals.h>
#include <Magnum/MeshTools/Copy.h>
#include <Magnum/MeshTools/Transform.h>

#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/AbstractTranslationRotation3D.h>
#include <Magnum/SceneGraph/Object.h>
#include <Magnum/SceneGraph/Scene.h>

#include "./octree/LooseOctree.h"
#include "./arcball/ArcBall.h"
#include "./arcball/ArcBallCamera.h"

#include "./objects/Skybox.h"
#include "./objects/Grid.h"

namespace Magnum

{
  using Object3D = SceneGraph::Object<SceneGraph::MatrixTransformation3D>;
  using Scene3D = SceneGraph::Scene<SceneGraph::MatrixTransformation3D>;

  using namespace Math::Literals;

  class RMD : public Platform::Application
  {
  public:
    explicit RMD(const Arguments &arguments);

  private:
    void drawEvent() override;
    void viewportEvent(ViewportEvent &event) override;
    void keyPressEvent(KeyEvent &event) override;
    void mousePressEvent(MouseEvent &event) override;
    void mouseReleaseEvent(MouseEvent &event) override;
    void mouseMoveEvent(MouseMoveEvent &event) override;
    void mouseScrollEvent(MouseScrollEvent &event) override;

    Scene3D _scene;
    SceneGraph::DrawableGroup3D _drawables;
    Containers::Optional<ArcBallCamera> _arcballCamera;

    bool _paused = false;
    bool _skipFrame = false;
  };
  RMD::RMD(const Arguments &arguments) : Platform::Application{arguments, NoCreate}
  {
    /* INFO Settings */
    Utility::Arguments args;
    args.addSkippedPrefix("magnum")
        .parse(arguments.argc, arguments.argv);
    /* INFO Window and parameters */
    {
      const Vector2 dpiScaling = this->dpiScaling({});
      Configuration conf;
      conf.setTitle("RMD v0.1.3.0")
          .setSize(conf.size(), dpiScaling)
          .setWindowFlags(Configuration::WindowFlag::Resizable);
      GLConfiguration glConf;
      glConf.setSampleCount(dpiScaling.max() < 2.0f ? 8 : 2);
      if (!tryCreate(conf, glConf))
      {
        create(conf, glConf.setSampleCount(0));
      }
    }
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

    /* INFO Background */
    {
      new Skybox(_scene, _drawables, 30.0f);
      new Grid(_scene, _drawables, 5.0f, Vector2i{16}, Color3{0.7f});
    }

    /* INFO Camera */
    {
      const Vector3 eye = Vector3::zAxis(5.0f);
      const Vector3 center{};
      const Vector3 up = Vector3::yAxis();
      const Deg fov = 45.0_degf;
      _arcballCamera.emplace(_scene, eye, center, up, fov, windowSize(), framebufferSize());
      _arcballCamera->setLagging(0.85f);
    }

    /* Loop at 60 Hz max (16)*/
    setSwapInterval(1);
    setMinimalLoopPeriod(16);
  }

  void RMD::drawEvent()
  {
    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color | GL::FramebufferClear::Depth);

    if (!_paused || _skipFrame)
    {
      _skipFrame = false;
    }
    /* Update camera */
    bool camChanged = _arcballCamera->update();
    _arcballCamera->draw(_drawables);
    swapBuffers();

    /* If the camera is moving or the animation is running, redraw immediately */
    if (camChanged)
      redraw();
  }

  void RMD::viewportEvent(ViewportEvent &event)
  {
    GL::defaultFramebuffer.setViewport({{}, event.framebufferSize()});
    _arcballCamera->reshape(event.windowSize(), event.framebufferSize());
  }

  void RMD::keyPressEvent(KeyEvent &event)
  {
    if (event.key() == KeyEvent::Key::B)
    {
      /*_drawBoundingBoxes ^= true;*/
    }
    else if (event.key() == KeyEvent::Key::R)
    {
      _arcballCamera->reset();
    }
    else if (event.key() == KeyEvent::Key::Space)
    {
      _paused ^= true;
    }
    else if (event.key() == KeyEvent::Key::Right)
    {
      _skipFrame = true;
    }
    else
      return;

    event.setAccepted();
    redraw();
  }

  void RMD::mousePressEvent(MouseEvent &event)
  {
    /* Enable mouse capture so the mouse can drag outside of the window */
    /** @todo replace once https://github.com/mosra/magnum/pull/419 is in */
    SDL_CaptureMouse(SDL_TRUE);
    _arcballCamera->initTransformation(event.position());
    event.setAccepted();
    redraw(); /* camera has changed, redraw! */
  }

  void RMD::mouseReleaseEvent(MouseEvent &)
  {
    /* Disable mouse capture again */
    /** @todo replace once https://github.com/mosra/magnum/pull/419 is in */
    SDL_CaptureMouse(SDL_FALSE);
  }

  void RMD::mouseMoveEvent(MouseMoveEvent &event)
  {
    if (!event.buttons())
      return;

    if (event.modifiers() & MouseMoveEvent::Modifier::Shift)
      _arcballCamera->translate(event.position());
    else
      _arcballCamera->rotate(event.position());

    event.setAccepted();
    redraw(); /* camera has changed, redraw! */
  }

  void RMD::mouseScrollEvent(MouseScrollEvent &event)
  {
    const Float delta = event.offset().y();
    if (Math::abs(delta) < 1.0e-2f)
      return;

    _arcballCamera->zoom(delta);

    event.setAccepted();
    redraw(); /* camera has changed, redraw! */
  }
}

MAGNUM_APPLICATION_MAIN(Magnum::RMD)