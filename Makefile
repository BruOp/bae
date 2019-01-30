help: ## Print each command with its help string
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort |\
	 awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'


build: ## Build stuff
	cmake --build build --config Debug --target bae -- -j 10

run: ## Run the build
	build/bae/bae
