#include <drivers/gpio.h>
#include <drivers/uart.h>
#include <drivers/timer.h>
#include <drivers/oled_ssd1306.h>
#include <interrupt_controller.h>

#include <memory/memmap.h>
#include <memory/kernel_heap.h>

#include <process/process_manager.h>

#include <fs/filesystem.h>

#include <stdstring.h>
#include <stdfile.h>

extern "C" void Timer_Callback()
{
	sProcessMgr.Schedule();
}

extern "C" unsigned char __idle_process[];
extern "C" unsigned char __proc_test_1[];
extern "C" unsigned char __proc_test_2[];
extern "C" unsigned int __idle_process_len;
extern "C" unsigned int __proc_test_1_len;
extern "C" unsigned int __proc_test_2_len;

extern "C" int _kernel_main(void)
{
	// inicializace souboroveho systemu
	sFilesystem.Initialize();

	// vytvoreni hlavniho systemoveho (idle) procesu
	sProcessMgr.Create_Process(__idle_process, __idle_process_len, true);

	// vytvoreni jednoho testovaciho procesu
	sProcessMgr.Create_Process(__proc_test_1, __proc_test_1_len, false);
	// vytvoreni druheho testovaciho procesu
	sProcessMgr.Create_Process(__proc_test_2, __proc_test_2_len, false);

	// zatim zakazeme IRQ casovace
	sInterruptCtl.Disable_Basic_IRQ(hal::IRQ_Basic_Source::Timer);

	// nastavime casovac - v callbacku se provadi planovani procesu
	sTimer.Enable(Timer_Callback, 0x100, NTimer_Prescaler::Prescaler_1);

	// povolime IRQ casovace
	sInterruptCtl.Enable_Basic_IRQ(hal::IRQ_Basic_Source::Timer);

	// povolime IRQ (nebudeme je maskovat) a od tohoto momentu je vse v rukou planovace
	sInterruptCtl.Set_Mask_IRQ(false);

	// vynutime prvni spusteni planovace
	sProcessMgr.Schedule();

	// tohle uz se mockrat nespusti - dalsi IRQ preplanuje procesor na nejaky z tasku (bud systemovy nebo uzivatelsky)
	while (true)
		;
	
	return 0;
}
