#ifndef __PRECIZA_GUI_H__
#define __PRECIZA_GUI_H__
#include "ui_PrecizaGUI.h"
class SCBPrecizaTracker;
class QTimer;
class QValidator;

class PrecizaGUI : public QMainWindow
{
	Q_OBJECT

public:
	PrecizaGUI(QWidget *parent = 0);
	~PrecizaGUI();

private:
	Ui_MainWindow* m_ui;
	SCBPrecizaTracker* m_tracker;
	QTimer* m_timerTool;
	QTimer* m_timerPoint;
	QValidator* m_validator;
	int m_currentTab;

private slots:
	void slotNewConnection();
	void slotStartTracking();
	void slotStopTracking();
	void slotGetToolTransform();
	void slotGetPointTransform();
};

#endif //!__PRECIZA_GUI_H__