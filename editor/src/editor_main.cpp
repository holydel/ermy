#include <iostream>
#include "ermy.h"

#include "ermy_entry_point.h"

class Editor : public ermy::Application
{
public:
	void OnConfigure() override
	{
		auto& logConfig = ermy::logger::Config();
		logConfig.FileMirroring.enabled = true;

		staticConfig.appName = "Ermy Editor";
	}
};

static Editor editor;