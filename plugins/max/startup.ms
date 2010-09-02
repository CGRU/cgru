(
	-- Load macros:
	macros.load (systemTools.getEnvVariable "AF_ROOT" + "/plugins/max")

	-- in interacive mode only
	if (IsNetServer() == false) then (  
		
		-- Create menu:
		local cgruMenu = menuMan.findMenu "CGRU" 

		-- Remove old menu:
		if (cgruMenu != undefined) then (menuMan.unRegisterMenu cgruMenu)

		menuMan.registerMenuContext 0x12345678

		local mainMenuBar = menuMan.getMainMenuBar()

		cgruMenu = menuMan.createMenu "CGRU"

		local cgruItemAfanasy = menuMan.createActionItem "Afanasy" "CGRU"

		local cgruItem = menuMan.createSubMenuItem "CGRU" cgruMenu

		-- main
		cgruMenu.addItem cgruItemAfanasy -1

		local subMenuIndex = mainMenuBar.numItems()
		mainMenuBar.addItem cgruItem subMenuIndex

		menuMan.updateMenuBar()
	)
)
