#include "pch.h"
#include "Core/Application.h"
int main() {

	ApplicationSpecification app_spec;
	Application* app = new Application(app_spec);
    app->Run();
    
    return 0;
}
