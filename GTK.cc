#include "Flux/Flux.hh"
#include "Flux/Input.hh"
#include "Flux/Log.hh"
#include "Flux/OpenGL/GLRenderer.hh"
#include "Flux/Renderer.hh"
#include "FluxGTK.hh"
#include "gdk/gdkkeysyms.h"
#include "gdkmm/device.h"
#include "gdkmm/event.h"
#include "gdkmm/types.h"
#include "glibmm/containers.h"
#include "gtkmm/eventbox.h"
#include "gtkmm/glarea.h"
#include "gtkmm/widget.h"
#include "sigc++/functors/mem_fun.h"
#include "sigc++/functors/ptr_fun.h"
#include <algorithm>
#include <map>

using namespace Flux::GLRenderer;

GLCtx* Flux::GLRenderer::current_window = nullptr;

void Flux::GLRenderer::createWindow(const int& width, const int& height, const std::string& title)
{
    // Do Nothing
}

static Gtk::GLArea* glarea = nullptr;
static Gtk::EventBox* eventbox = nullptr;

void onRealize()
{
    glarea->make_current();

    if (!gladLoadGL())
    {
        LOG_ERROR("Could not load OpenGL");
        exit(-1);
    }

    Flux::GLRenderer::_startGL();

    glViewport(0, 0, Flux::GLRenderer::current_window->width, Flux::GLRenderer::current_window->height);
}

void (*func)() = nullptr;

void Flux::setMainLoopFunction(void (*fun)())
{
    func = fun;
}

void render()
{
    func();
}

static std::vector<float> scroll_offset { 0 };

bool onRender(const Glib::RefPtr<Gdk::GLContext>& context)
{
    render();
    glarea->queue_render();
    scroll_offset[0] = 0;
    current_window->offset = glm::vec2(0, 0);

    return true;
}

void Flux::runMainloop()
{
}

void onResize(int width, int height)
{
    Flux::GLRenderer::current_window->width = width;
    Flux::GLRenderer::current_window->height = height;
    glViewport(0, 0, width, height);

    render();
}

bool onKeyPress(GdkEventKey* event);
bool onKeyRelease(GdkEventKey* event);

bool onMouseButtonPress(GdkEventButton* event);
bool onMouseButtonRelease(GdkEventButton* event);

bool onMouseMotion(GdkEventMotion* event);
bool onScroll(GdkEventScroll* event)
{
    if (event->direction == GDK_SCROLL_UP)
    {
        scroll_offset[0] += 1;
    }
    else if (event->direction == GDK_SCROLL_DOWN)
    {
        scroll_offset[0] -= 1;
    }
    return true;
}

void Flux::GTK::startRenderer(Gtk::EventBox* event_box, Gtk::GLArea* gl_area)
{
    glarea = gl_area;
    eventbox = event_box;

    int ma = 3, mi = 3;
    gl_area->set_required_version(ma, mi);

    GLCtx* gctx = new GLCtx;
    current_window = gctx;

    gctx->width = gl_area->get_width();
    gctx->height = gl_area->get_height();
    gctx->title = "bababab";
    gctx->mouse_mode = Input::MouseMode::Free;
    gctx->offset = glm::vec2(0, 0);
    gctx->mouse_pos = glm::vec2(0, 0);

    gl_area->set_auto_render(true);

    // Add all the needed signals
    gl_area->signal_realize().connect(sigc::ptr_fun(&onRealize));
    gl_area->signal_render().connect(sigc::ptr_fun(&onRender));
    gl_area->signal_resize().connect(sigc::ptr_fun(&onResize));

    event_box->add_events(Gdk::BUTTON_MOTION_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::KEY_PRESS_MASK |
                          Gdk::SMOOTH_SCROLL_MASK | Gdk::POINTER_MOTION_MASK | Gdk::SCROLL_MASK);
    event_box->signal_key_press_event().connect(sigc::ptr_fun(&onKeyPress), false);
    event_box->signal_key_release_event().connect(sigc::ptr_fun(&onKeyRelease), false);
    event_box->signal_button_press_event().connect(sigc::ptr_fun(&onMouseButtonPress), false);
    event_box->signal_button_release_event().connect(sigc::ptr_fun(&onMouseButtonRelease), false);
    event_box->signal_scroll_event().connect(sigc::ptr_fun(&onScroll), false);

    gl_area->grab_focus();
    event_box->grab_focus();
    event_box->grab_default();
    eventbox->set_focus_on_click(true);

    // How to get position in parent
    int x, y;
    Gtk::Widget* parent = event_box->get_toplevel();
    event_box->translate_coordinates(*parent, 0, 0, x, y);

    // Mouse events
    parent->signal_motion_notify_event().connect(sigc::ptr_fun(&onMouseMotion), true);

    gtk_grab_add((GtkWidget*)event_box->gobj());
}

// TODO: All these functions
bool Flux::GLRenderer::_windowStartFrame()
{
    return true;
}

void Flux::GLRenderer::_windowEndFrame()
{
}

void Flux::GLRenderer::destroyWindow()
{
}

double Flux::Renderer::getTime()
{
    return std::time(0);
}

int gtkKeyToFluxKey(int key);

static std::map<int, bool> keys_pressed;

bool onKeyPress(GdkEventKey* event)
{
    keys_pressed[gtkKeyToFluxKey(event->keyval)] = true;
    return true;
}

bool onKeyRelease(GdkEventKey* event)
{
    keys_pressed[gtkKeyToFluxKey(event->keyval)] = false;
    return true;
}

static std::map<int, bool> buttons_pressed;

bool onMouseButtonPress(GdkEventButton* event)
{
    buttons_pressed[event->button] = true;
    return true;
}

bool onMouseButtonRelease(GdkEventButton* event)
{
    buttons_pressed[event->button] = false;
    return true;
}

bool grabbed = true;

bool onMouseMotion(GdkEventMotion* event)
{
    // How to get position in parent
    // int x, y;
    // Gtk::Widget* parent = eventbox->get_toplevel();
    // eventbox->translate_coordinates(*parent, 0, 0, x, y);

    glm::vec2 new_pos = glm::vec2(event->x, event->y);


    // TODO: Figure out gtkmm way of doing this
    if (!(gtk_get_event_widget((GdkEvent*)event) == (GtkWidget*)eventbox->gobj()))
    {

        // It's not over the glarea
        if (grabbed)
        {
            gtk_grab_remove((GtkWidget*)eventbox->gobj());
            grabbed = false;
        }
        current_window->offset = glm::vec2(0, 0);
    }
    else
    {

        // It is over the glarea
        if (!grabbed)
        {
            gtk_grab_add((GtkWidget*)eventbox->gobj());
            grabbed = true;
            glarea->grab_focus();
            eventbox->grab_focus();
        }

        current_window->offset = current_window->mouse_pos - new_pos;
        current_window->mouse_pos = new_pos;

        // Check for mouse modes
        // As of right now, Mouse Mode confined isn't supported on GTK
        // Nor is hiding the cursor
        if (current_window->mouse_mode == Flux::Input::MouseMode::Confined)
        {
            if (new_pos.x < 15)
            {
                // Get global positon of window
                int total_x, total_y;
                eventbox->get_window()->get_origin(total_x, total_y);

                // Set position and move mouse pointer
                new_pos.x = current_window->width-15;
                gdk_device_warp(event->device, eventbox->get_screen()->gobj(), total_x +  new_pos.x, total_y +  new_pos.y);
            }
            else if (new_pos.x > current_window->width-15)
            {
                // Get global positon of window
                int total_x, total_y;
                eventbox->get_window()->get_origin(total_x, total_y);

                // Set position and move mouse pointer
                new_pos.x = 15;
                gdk_device_warp(event->device, eventbox->get_screen()->gobj(), total_x +  new_pos.x, total_y +  new_pos.y);
            }

            if (new_pos.y > current_window->height-15)
            {
                // Get global positon of window
                int total_x, total_y;
                eventbox->get_window()->get_origin(total_x, total_y);

                // Set position and move mouse pointer
                new_pos.y = 15;
                gdk_device_warp(event->device, eventbox->get_screen()->gobj(), total_x +  new_pos.x, total_y +  new_pos.y);
            }
            else if (new_pos.y < 15)
            {
                // Get global positon of window
                int total_x, total_y;
                eventbox->get_window()->get_origin(total_x, total_y);

                // Set position and move mouse pointer
                new_pos.y = current_window->height-15;
                gdk_device_warp(event->device, eventbox->get_screen()->gobj(), total_x +  new_pos.x, total_y +  new_pos.y);
            }
            
        }
    }


    return true;
}

bool Flux::Input::isKeyPressed(int key)
{

    try
    {
        return keys_pressed.at(key);
    }
    catch (std::exception& e)
    {
        return false;
    }
}

bool Flux::Input::isMouseButtonPressed(int button)
{
    try
    {
        return buttons_pressed.at(button);
    }
    catch (std::exception& e)
    {
        return false;
    }
}

void Flux::Input::setCursorMode(Input::CursorMode mode)
{
    LOG_WARN("Unimplemented :/");
}

void Flux::Input::setMouseMode(Input::MouseMode mode)
{
    current_window->mouse_mode = mode;
}

glm::vec2& Flux::Input::getMouseOffset()
{
    return current_window->offset;
}

glm::vec2& Flux::Input::getMousePosition()
{
    return current_window->mouse_pos;
}

float Flux::Input::getScrollWheelOffset()
{
    return (float)scroll_offset[0];
}

/// ======================================
/// Keys

int gtkKeyToFluxKey(int key)
{
    switch (key)
    {
    case (GDK_KEY_a):
        return FLUX_KEY_A;
    case (GDK_KEY_b):
        return FLUX_KEY_B;
    case (GDK_KEY_c):
        return FLUX_KEY_C;
    case (GDK_KEY_d):
        return FLUX_KEY_D;
    case (GDK_KEY_e):
        return FLUX_KEY_E;
    case (GDK_KEY_f):
        return FLUX_KEY_F;
    case (GDK_KEY_g):
        return FLUX_KEY_G;
    case (GDK_KEY_h):
        return FLUX_KEY_H;
    case (GDK_KEY_i):
        return FLUX_KEY_I;
    case (GDK_KEY_j):
        return FLUX_KEY_J;
    case (GDK_KEY_k):
        return FLUX_KEY_K;
    case (GDK_KEY_l):
        return FLUX_KEY_L;
    case (GDK_KEY_m):
        return FLUX_KEY_M;
    case (GDK_KEY_n):
        return FLUX_KEY_N;
    case (GDK_KEY_o):
        return FLUX_KEY_O;
    case (GDK_KEY_p):
        return FLUX_KEY_P;
    case (GDK_KEY_q):
        return FLUX_KEY_Q;
    case (GDK_KEY_r):
        return FLUX_KEY_R;
    case (GDK_KEY_s):
        return FLUX_KEY_S;
    case (GDK_KEY_t):
        return FLUX_KEY_T;
    case (GDK_KEY_u):
        return FLUX_KEY_U;
    case (GDK_KEY_v):
        return FLUX_KEY_V;
    case (GDK_KEY_w):
        return FLUX_KEY_W;
    case (GDK_KEY_x):
        return FLUX_KEY_X;
    case (GDK_KEY_y):
        return FLUX_KEY_Y;
    case (GDK_KEY_z):
        return FLUX_KEY_Z;
    case (GDK_KEY_A):
        return FLUX_KEY_A;
    case (GDK_KEY_B):
        return FLUX_KEY_B;
    case (GDK_KEY_C):
        return FLUX_KEY_C;
    case (GDK_KEY_D):
        return FLUX_KEY_D;
    case (GDK_KEY_E):
        return FLUX_KEY_E;
    case (GDK_KEY_F):
        return FLUX_KEY_F;
    case (GDK_KEY_G):
        return FLUX_KEY_G;
    case (GDK_KEY_H):
        return FLUX_KEY_H;
    case (GDK_KEY_I):
        return FLUX_KEY_I;
    case (GDK_KEY_J):
        return FLUX_KEY_J;
    case (GDK_KEY_K):
        return FLUX_KEY_K;
    case (GDK_KEY_L):
        return FLUX_KEY_L;
    case (GDK_KEY_M):
        return FLUX_KEY_M;
    case (GDK_KEY_N):
        return FLUX_KEY_N;
    case (GDK_KEY_O):
        return FLUX_KEY_O;
    case (GDK_KEY_P):
        return FLUX_KEY_P;
    case (GDK_KEY_Q):
        return FLUX_KEY_Q;
    case (GDK_KEY_R):
        return FLUX_KEY_R;
    case (GDK_KEY_S):
        return FLUX_KEY_S;
    case (GDK_KEY_T):
        return FLUX_KEY_T;
    case (GDK_KEY_U):
        return FLUX_KEY_U;
    case (GDK_KEY_V):
        return FLUX_KEY_V;
    case (GDK_KEY_W):
        return FLUX_KEY_W;
    case (GDK_KEY_X):
        return FLUX_KEY_X;
    case (GDK_KEY_Y):
        return FLUX_KEY_Y;
    case (GDK_KEY_Z):
        return FLUX_KEY_Z;
    case (GDK_KEY_F1):
        return FLUX_KEY_F1;
    case (GDK_KEY_F2):
        return FLUX_KEY_F2;
    case (GDK_KEY_F3):
        return FLUX_KEY_F3;
    case (GDK_KEY_F4):
        return FLUX_KEY_F4;
    case (GDK_KEY_F5):
        return FLUX_KEY_F5;
    case (GDK_KEY_F6):
        return FLUX_KEY_F6;
    case (GDK_KEY_F7):
        return FLUX_KEY_F7;
    case (GDK_KEY_F8):
        return FLUX_KEY_F8;
    case (GDK_KEY_F9):
        return FLUX_KEY_F9;
    case (GDK_KEY_F10):
        return FLUX_KEY_F10;
    case (GDK_KEY_F11):
        return FLUX_KEY_F11;
    case (GDK_KEY_F12):
        return FLUX_KEY_F12;
    case (GDK_KEY_F13):
        return FLUX_KEY_F13;
    case (GDK_KEY_F14):
        return FLUX_KEY_F14;
    case (GDK_KEY_F15):
        return FLUX_KEY_F15;
    case (GDK_KEY_F16):
        return FLUX_KEY_F16;
    case (GDK_KEY_F17):
        return FLUX_KEY_F17;
    case (GDK_KEY_F18):
        return FLUX_KEY_F18;
    case (GDK_KEY_F19):
        return FLUX_KEY_F19;
    case (GDK_KEY_F20):
        return FLUX_KEY_F20;
    case (GDK_KEY_F21):
        return FLUX_KEY_F21;
    case (GDK_KEY_F22):
        return FLUX_KEY_F22;
    case (GDK_KEY_F23):
        return FLUX_KEY_F23;
    case (GDK_KEY_F24):
        return FLUX_KEY_F24;
    case (GDK_KEY_F25):
        return FLUX_KEY_F25;
    case (GDK_KEY_0):
        return FLUX_KEY_0;
    case (GDK_KEY_1):
        return FLUX_KEY_1;
    case (GDK_KEY_2):
        return FLUX_KEY_2;
    case (GDK_KEY_3):
        return FLUX_KEY_3;
    case (GDK_KEY_4):
        return FLUX_KEY_4;
    case (GDK_KEY_5):
        return FLUX_KEY_5;
    case (GDK_KEY_6):
        return FLUX_KEY_6;
    case (GDK_KEY_7):
        return FLUX_KEY_7;
    case (GDK_KEY_8):
        return FLUX_KEY_8;
    case (GDK_KEY_9):
        return FLUX_KEY_9;
    case (GDK_KEY_space):
        return FLUX_KEY_SPACE;
    case (GDK_KEY_apostrophe):
        return FLUX_KEY_APOSTROPHE;
    case (GDK_KEY_comma):
        return FLUX_KEY_COMMA;
    case (GDK_KEY_minus):
        return FLUX_KEY_MINUS;
    case (GDK_KEY_period):
        return FLUX_KEY_PERIOD;
    case (GDK_KEY_slash):
        return FLUX_KEY_SLASH;
    case (GDK_KEY_semicolon):
        return FLUX_KEY_SEMICOLON;
    case (GDK_KEY_equal):
        return FLUX_KEY_EQUAL;
    case (GDK_KEY_Escape):
        return FLUX_KEY_ESCAPE;
    case (GDK_KEY_Return):
        return FLUX_KEY_RETURN;
    case (GDK_KEY_Tab):
        return FLUX_KEY_TAB;
    case (GDK_KEY_BackSpace):
        return FLUX_KEY_BACKSPACE;
    case (GDK_KEY_Insert):
        return FLUX_KEY_INSERT;
    case (GDK_KEY_Delete):
        return FLUX_KEY_DELETE;
    case (GDK_KEY_Right):
        return FLUX_KEY_RIGHT;
    case (GDK_KEY_Left):
        return FLUX_KEY_LEFT;
    case (GDK_KEY_Down):
        return FLUX_KEY_DOWN;
    case (GDK_KEY_Up):
        return FLUX_KEY_UP;
    case (GDK_KEY_Page_Up):
        return FLUX_KEY_PAGE_UP;
    case (GDK_KEY_Page_Down):
        return FLUX_KEY_PAGE_DOWN;
    case (GDK_KEY_Home):
        return FLUX_KEY_HOME;
    case (GDK_KEY_End):
        return FLUX_KEY_END;
    case (GDK_KEY_Caps_Lock):
        return FLUX_KEY_CAPS_LOCK;
    case (GDK_KEY_Scroll_Lock):
        return FLUX_KEY_SCROLL_LOCK;
    case (GDK_KEY_Num_Lock):
        return FLUX_KEY_NUM_LOCK;
    case (GDK_KEY_Pause):
        return FLUX_KEY_PAUSE;
    case (GDK_KEY_KP_0):
        return FLUX_KEY_KP_0;
    case (GDK_KEY_KP_1):
        return FLUX_KEY_KP_1;
    case (GDK_KEY_KP_2):
        return FLUX_KEY_KP_2;
    case (GDK_KEY_KP_3):
        return FLUX_KEY_KP_3;
    case (GDK_KEY_KP_4):
        return FLUX_KEY_KP_4;
    case (GDK_KEY_KP_5):
        return FLUX_KEY_KP_5;
    case (GDK_KEY_KP_6):
        return FLUX_KEY_KP_6;
    case (GDK_KEY_KP_7):
        return FLUX_KEY_KP_7;
    case (GDK_KEY_KP_8):
        return FLUX_KEY_KP_8;
    case (GDK_KEY_KP_9):
        return FLUX_KEY_KP_9;
    case (GDK_KEY_KP_Decimal):
        return FLUX_KEY_KP_DECIMAL;
    case (GDK_KEY_KP_Divide):
        return FLUX_KEY_KP_DIVIDE;
    case (GDK_KEY_KP_Multiply):
        return FLUX_KEY_KP_MULTIPLY;
    case (GDK_KEY_KP_Subtract):
        return FLUX_KEY_KP_SUBTRACT;
    case (GDK_KEY_KP_Add):
        return FLUX_KEY_KP_ADD;
    case (GDK_KEY_KP_Enter):
        return FLUX_KEY_KP_ENTER;
    case (GDK_KEY_KP_Equal):
        return FLUX_KEY_KP_EQUAL;
    case (GDK_KEY_Menu):
        return FLUX_KEY_MENU;

    // Modifiers
    case (GDK_KEY_Shift_L):
        return FLUX_KEY_LEFT_SHIFT;
    case (GDK_KEY_Shift_R):
        return FLUX_KEY_RIGHT_SHIFT;

    case (GDK_KEY_Control_L):
        return FLUX_KEY_LEFT_CONTROL;
    case (GDK_KEY_Control_R):
        return FLUX_KEY_RIGHT_CONTROL;

    case (GDK_KEY_Alt_L):
        return FLUX_KEY_LEFT_ALT;
    case (GDK_KEY_Alt_R):
        return FLUX_KEY_RIGHT_ALT;

    case (GDK_KEY_Super_L):
        return FLUX_KEY_LEFT_SUPER;
    case (GDK_KEY_Super_R):
        return FLUX_KEY_RIGHT_SUPER;
    }

    // TODO: Something
    return -1;
}