build:
	cmake --build build --config Debug --target bae -- -j 10

run:
	build/bae/bae
