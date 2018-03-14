#include "PrecizaGUI.h"
#include "SCBPrecizaTracker.h"

#include <QTimer>
#include <QDoubleValidator>

#include <vtkSmartPointer.h>
#include <vtkPoints.h>

PrecizaGUI::PrecizaGUI(QWidget *parent)
	: QMainWindow(parent)
{
	this->m_ui = new Ui_MainWindow;
	this->m_ui->setupUi(this);

	this->m_ui->pushButton_start->setEnabled(false);
	this->m_ui->pushButton_stop->setEnabled(false);
	this->m_ui->label_NoTool->setVisible(false);
	this->m_validator = new QDoubleValidator(this);

	this->m_tracker = new SCBPrecizaTracker();
	this->m_tracker->setupCameras();

	this->m_timerTool  = new QTimer(this);
	this->m_timerPoint = new QTimer(this);

	connect(m_tracker, SIGNAL(signalIsConnected()), this, SLOT(slotNewConnection()));
	connect(this->m_ui->pushButton_start, SIGNAL(released()), this, SLOT(slotStartTracking()));
	connect(this->m_ui->pushButton_stop, SIGNAL(released()), this, SLOT(slotStopTracking()));
	connect(m_timerTool, SIGNAL(timeout()), this, SLOT(slotGetToolTransform()));
	connect(m_timerPoint, SIGNAL(timeout()), this, SLOT(slotGetPointTransform()));
}

PrecizaGUI::~PrecizaGUI()
{
	delete this->m_timerPoint;
	delete this->m_timerTool;
	delete this->m_validator;
	delete this->m_ui;
	delete this->m_tracker;
}


void PrecizaGUI::slotNewConnection()
{
	this->m_ui->pushButton_start->setEnabled(true);	
	this->m_ui->textBrowser_output->append("Tracker is connected. \n");
}


void PrecizaGUI::slotStartTracking()
{
	this->m_tracker->startTracking();
	this->m_ui->pushButton_stop->setEnabled(true);
	this->m_ui->pushButton_start->setEnabled(false);
	//this->m_timer->start(20);
	
	m_currentTab = this->m_ui->tabWidget->currentIndex();
	if (m_currentTab == 0)
	{
		//connect(this->m_tracker, &SCBPrecizaTracker::signalCommandUpdated, this, &PrecizaGUI::slotGetToolTransform);
		m_timerTool->start(10);
		this->m_ui->textBrowser_output->append("Start tool tracking.... \n");
	}
	if (m_currentTab == 1)
	{
		//connect(this->m_tracker, &SCBPrecizaTracker::signalCommandUpdated, this, &PrecizaGUI::slotGetPointTransform);
		m_timerPoint->start(10);
		this->m_ui->textBrowser_output->append("Start point tracking.... \n");
	}
}

void PrecizaGUI::slotStopTracking()
{
	this->m_tracker->stopTracking();
	this->m_ui->pushButton_stop->setEnabled(false);
	this->m_ui->pushButton_start->setEnabled(true);
	//this->m_timer->stop();
	if (m_currentTab == 0)
	{
		//disconnect(this->m_tracker, &SCBPrecizaTracker::signalCommandUpdated, this, &PrecizaGUI::slotGetToolTransform);
		m_timerTool->stop();
		this->m_ui->textBrowser_output->append("Stop tool tracking.... \n");
	}
	if (m_currentTab == 1)
	{
		//disconnect(this->m_tracker, &SCBPrecizaTracker::signalCommandUpdated, this, &PrecizaGUI::slotGetPointTransform);
		m_timerPoint->stop();
		this->m_ui->textBrowser_output->append("Stop point tracking.... \n");
	}
}

void PrecizaGUI::slotGetToolTransform()
{
	this->m_ui->label_NoTool->setVisible(false);
	double pos[3] = { 0, 0, 0 };
	double ori[4] = { 0, 0, 0, 0 };

	if (this->m_tracker->getTransformMatrix(pos, ori) == 0)
	{
		m_ui->textBrowser_Tx->setText(QString::number(pos[0]));
		m_ui->textBrowser_Ty->setText(QString::number(pos[1]));
		m_ui->textBrowser_Tz->setText(QString::number(pos[2]));
		//ui.label_vale->setText(rms_error);
		m_ui->textBrowser_Q0->setText(QString::number(ori[0]));
		m_ui->textBrowser_Qx->setText(QString::number(ori[1]));
		m_ui->textBrowser_Qy->setText(QString::number(ori[2]));
		m_ui->textBrowser_Qz->setText(QString::number(ori[3]));
	}
	else
	{
		this->m_ui->label_NoTool->setVisible(true);
	}
}

//void PrecizaGUI::slotGetToolTransform()
//{
//	this->m_tracker->sendMessage("req2");
//	QString message = this->m_tracker->getMessage();
//	QString toolName = "vspTool2";
//	int toolPos = message.indexOf(toolName);
//	if (toolPos == -1)
//	{
//		m_ui->textBrowser_Tx->setText("");
//		m_ui->textBrowser_Ty->setText("");
//		m_ui->textBrowser_Tz->setText("");
//		m_ui->textBrowser_Q0->setText("");
//		m_ui->textBrowser_Qx->setText("");
//		m_ui->textBrowser_Qy->setText("");
//		m_ui->textBrowser_Qz->setText("");
//		this->m_ui->label_NoTool->setVisible(true);
//		return;
//	}
//	this->m_ui->label_NoTool->setVisible(false);
//	QString xtag = "x";
//	QString ytag = "y";
//	QString ztag = "z";
//	QString rtag = "rms";
//	QString q0tag = "q0";
//	QString q1tag = "q1";
//	QString q2tag = "q2";
//	QString q3tag = "q3";
//	QString etag = "vep";
//	int xpos = message.indexOf(xtag);
//	QString ss_from_x;
//	ss_from_x = message.mid(xpos);
//
//	int ypos = ss_from_x.indexOf(ytag);
//	int zpos = ss_from_x.indexOf(ztag);
//	int rpos = ss_from_x.indexOf(rtag);
//	int q0pos = ss_from_x.indexOf(q0tag);
//	int q1pos = ss_from_x.indexOf(q1tag);
//	int q2pos = ss_from_x.indexOf(q2tag);
//	int q3pos = ss_from_x.indexOf(q3tag);
//	int epos = ss_from_x.indexOf(etag);
//	//  +1: to eliminate "x";
//	QString xcoordinate = ss_from_x.mid(1, ypos - 1);
//	QString ycoordinate = ss_from_x.mid(ypos + 1, zpos - (ypos + 1));
//	QString zcoordinate = ss_from_x.mid(zpos + 1, rpos - (zpos + 1));
//	QString rms_error = ss_from_x.mid(rpos + 3, q0pos - (rpos + 3));
//	QString q0 = ss_from_x.mid(q0pos + 2, q1pos - (q0pos + 2));
//	QString q1 = ss_from_x.mid(q1pos + 2, q2pos - (q1pos + 2));
//	QString q2 = ss_from_x.mid(q2pos + 2, q3pos - (q2pos + 2));
//	QString q3 = ss_from_x.mid(q3pos + 2, epos - (q3pos + 2));
//
//	m_ui->textBrowser_Tx->setText(xcoordinate);
//	m_ui->textBrowser_Ty->setText(ycoordinate);
//	m_ui->textBrowser_Tz->setText(zcoordinate);
//	//ui.label_vale->setText(rms_error);
//	m_ui->textBrowser_Q0->setText(q0);
//	m_ui->textBrowser_Qx->setText(q1);
//	m_ui->textBrowser_Qy->setText(q2);
//	m_ui->textBrowser_Qz->setText(q3);
//}



void PrecizaGUI::slotGetPointTransform()
{
	vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
	if (this->m_tracker->getStrayMarkers(points) == 0)
	{
		this->m_ui->tableWidget->setRowCount(points->GetNumberOfPoints());
		for (int i = 0; i < points->GetNumberOfPoints(); i++)
		{
			double currPoint[3];
			points->GetPoint(i, currPoint);
			this->m_ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(currPoint[0])));
			this->m_ui->tableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(currPoint[1])));
			this->m_ui->tableWidget->setItem(i, 2, new QTableWidgetItem(QString::number(currPoint[2])));
		}
	}
	else
	{
		this->m_ui->tableWidget->clearContents();
	}
}

//"vspMarkersCNT3m1x100.00m1y101.01m1z1010.10m2x200.00m2y202.02m2z2020.20m3x300.00m3y301.01m3z3030.30vep"
//void PrecizaGUI::slotGetPointTransform() 
//{
//	this->m_tracker->sendMessage("reqsm");
//	QString message = this->m_tracker->getMessage();
//	QString pointStart = "vspMarkersCNT";
//
//	int startPos = message.indexOf(pointStart);
//	
//	if (startPos == -1)
//	{
//		this->m_ui->tableWidget->clearContents();
//		return;
//	}
//	int pos = message.indexOf("m1x");
//	int numberOfPoints = message.mid(13, pos-13).toInt();
//	this->m_ui->tableWidget->setRowCount(numberOfPoints);
//	
//
//	QString remainMessage;
//	remainMessage = message;
//
//	for (int i = 1; i < numberOfPoints + 1; i++)
//	{
//		QString numbertag = QString::number(i);
//		QString xtag = numbertag + QString("x");
//		QString ytag = numbertag + QString("y");
//		QString ztag = numbertag + QString("z");
//		QString endtag;
//		int xpos = remainMessage.indexOf(xtag);
//		remainMessage = remainMessage.mid(xpos);
//		int ypos = remainMessage.indexOf(ytag);
//		int zpos = remainMessage.indexOf(ztag);
//		if (i == numberOfPoints)
//		{
//			endtag = "vep";
//		}
//		else
//		{
//			endtag = QString::number(i + 1).append("x");
//		}
//		int epos = remainMessage.indexOf(endtag);
//
//		QString xcoordinate = remainMessage.mid(2, ypos - 2);
//		QString ycoordinate = remainMessage.mid(ypos + 2, zpos - (ypos + 2));
//		QString zcoordinate = remainMessage.mid(zpos + 2, epos - (zpos + 2));
//
//		this->m_ui->tableWidget->setItem(i - 1, 0, new QTableWidgetItem(xcoordinate));
//		this->m_ui->tableWidget->setItem(i - 1, 1, new QTableWidgetItem(ycoordinate));
//		this->m_ui->tableWidget->setItem(i - 1, 2, new QTableWidgetItem(zcoordinate));
//	}
//}