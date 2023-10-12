#include "TestRenderer.hpp"

int main() {
	TestRenderer renderer;
	renderer.Start();
	while (renderer.Update()) {
		continue;
	}
	renderer.Stop();

	return 0;
}