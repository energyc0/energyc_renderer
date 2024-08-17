#include "RendererApplication.h"

#define MY_MACRO(...) std::cout <<__FILE__ <<__LINE__ << __VA_ARGS__ <<

int main(){
    RendererApplication application(800, 600, "energyc_renderer", "energyc_renderer");
    application.run();
    return 0;
}