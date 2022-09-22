//
/*
#Brief Introduction:
	This a timer to help me sink in study
#Function: 
	😊[1]Select a time from preset time list use to count down  
	no[2]Create a timer preset and put it into preset time list 
	yes[3]show the time of counting down 
	yes[4]directly edit time to count down
	yes[5]start countdown 
	yes[6]pause countdown 
	yes[7]continue countdwon 
 	yes[8]reset countdown 
	yes[9]show a system tray icon with tip information about short cut keys
	yes[10]LButton click system tray icon will pop up a menu , which is constituted by window, to exit program. 
	yes[11]double click system tray to show or hide main window of timer.
	yes[12]use short cut keys to start/stop/continue/reset timer.
	yes[13]start/stop/continue/reset operation will pop a short time window to show the operation. maybe,play music to respond the corresponding operation also a good way 
	no[14]when timer is counting down(runing/stop) ,per five secs to store timer data until timer end
#What I need:
	*Data:
		+Memory Data
			Globle:	
				INT timerS tatus;//0=initial 1=runing 2=pause 3=finish
				typedef struct TimeOfTimer{ UINT status;CHAR hours;CHAR minutes;CHAR seconds;}TOT,*PTOT;//TOT theTime
				HINSTANCE instance;//program instance handle
				HWND hMainWindow;//main window of timer
				HWND hMenuWindow;//system tray icon menu window
				HWND hTipWindow;
				*deadLine、leftTime
				

		+File Data
			.preset[UTF-16]: content format like [xx:xx:xx] to store preset time list  to set time of timer.
				detail content example:
					[**:**:**
					 **:**:**
					 **:**:**
					]
					remark :end with \n,it's must
			status.txt: it's used to store timer status,including is timer runing or stoped and  the left time of  counting down
				detail:
					line 1: run(1) or stop(2)
					line 2: [xx:xx:xx]

	*UI:
		+MainWindow
			a combobox used to select time of timer
			a button to create preset time (use current time of timer, before creating,check the whether the same data has existed)
			Three updown control
			Two Label control
			TwoButton start/stop/continue,Reset
		+MenuWindow
			a button to exit program
		+TipWindow
			to show Short cut operation tip and timer finish notification
	*Program: 
		
*/