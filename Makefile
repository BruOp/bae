help: ## Print each command with its help string
	@grep -E '^[a-zA-Z_-]+:.*?## .*$$' $(MAKEFILE_LIST) | sort |\
	 awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-20s\033[0m %s\n", $$1, $$2}'


setup: ## Build stuff
	deps/bx/tools/bin/windows/genie.exe --file=scripts/genie.lua vs2017

compile_shaders:
	cd shaders && make TARGET=$(TARGET)
