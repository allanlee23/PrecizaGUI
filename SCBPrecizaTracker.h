#ifndef __SCB_PRECIZA_TRACKER_H__
#define __SCB_PRECIZA_TRACKER_H__


//Qt
#include <QObject>
#include <QtNetwork>
#include <QTcpServer>
class QTimer;

//vtk
class vtkMatrix4x4;
class vtkPoints;

class SCBPrecizaTracker : public QObject
{
	Q_OBJECT
public:
	explicit SCBPrecizaTracker(QObject* parent = NULL);
	~SCBPrecizaTracker() {};
	int setupCameras();
	void startTracking();
	void stopTracking();

	int getTransformMatrix(vtkMatrix4x4* output);
	int getTransformMatrix(double* position, double* quaterion);
	int getStrayMarkers(vtkPoints* points);
	QString getMessage();
	void sendMessage(char*);
	int getRespond();

signals:
	void signalIsConnected();
	void signalCommandUpdated();

private slots:
	void newConnect();
	void readMessage();
	void debugMessage();
	void socketDisconnected();

private:
	void convertQuatToRotationMatrix(double* q, vtkMatrix4x4* matrix);
	QTcpServer *m_tcpServer;
	QTcpSocket *m_tcpSocket;
	QTimer *m_timer;
	QString m_szHandleInformation;
	bool m_isConnected;
	bool m_isTracking;
};

#endif //!__SCB_PRECIZA_TRACKER_H__