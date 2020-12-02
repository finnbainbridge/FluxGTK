#ifndef FLUX_GTK_HH
#define FLUX_GTK_HH

#include "gtkmm/glarea.h"
#include <gtkmm.h>

namespace Flux { namespace GTK {

    /**
    Initializes Flux in a Gtk GLArea.
    The GLArea must be inside an EventBox
    */
    void startRenderer(Gtk::EventBox *event_box, Gtk::GLArea* gl_area);

}}

#endif