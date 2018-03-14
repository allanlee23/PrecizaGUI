#include "SCBPrecizatracker.h"

#include <vtkMatrix4x4.h>
#include <vtkSmartPointer.h>
#include <vtkPoints.h>

SCBPrecizaTracker::SCBPrecizaTracker(QObject* parent)
	:QObject(parent)
{
	m_isConnected = false;
	m_isTracking = false;
	m_szHandleInformation = "";
}

int SCBPrecizaTracker::setupCameras()
{
	m_tcpServer = new QTcpServer(this);
	m_tcpServer->listen(QHostAddress::Any, 1234); //listen port 1234
	connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(newConnect()));
	return 0;
}

void SCBPrecizaTracker::newConnect()
{
	m_isConnected = true;
	emit signalIsConnected();
	m_tcpSocket = m_tcpServer->nextPendingConnection();  
	qDebug() << m_tcpSocket->peerAddress();

	connect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
	//connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(readMessage())); 
	//connect(m_tcpSocket, SIGNAL(readyRead()), this, SLOT(debugMessage()));
}

void SCBPrecizaTracker::socketDisconnected()
{
	m_isConnected = false;
	qDebug() << "SOCKET DISCONNECT!!! ip: " << m_tcpSocket->peerAddress();
	disconnect(m_tcpSocket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
	m_tcpSocket->deleteLater();	
}

void SCBPrecizaTracker::readMessage() 
{
	QByteArray qba = m_tcpSocket->readAll(); 
	qDebug() << qba;
	m_szHandleInformation = QVariant(qba).toString();
	emit signalCommandUpdated();
}

void SCBPrecizaTracker::debugMessage()
{
	//QByteArray qba = m_tcpSocket->read
	//qDebug() << qba;
}

QString SCBPrecizaTracker::getMessage()
{
	return m_szHandleInformation;
}

void SCBPrecizaTracker::sendMessage(char* command)
{
	int bytewritten = m_tcpSocket->write(command, strlen(command));
	qDebug() << "Byte write: " << bytewritten;
}

int SCBPrecizaTracker::getRespond()
{
	bool bDone = false;
	//bool signalConnected = false;
	m_szHandleInformation = "";

	time_t currenttime,	starttime;

	time(&starttime);

	do 
	{
		time(&currenttime);
		if (difftime(currenttime, starttime) >= 5)
		{
			return 1;
		}
		
		if (m_tcpSocket->waitForReadyRead(1000) || (m_tcpSocket->bytesAvailable() > 0))
		{
			qDebug() << "byte Available: " << m_tcpSocket->bytesAvailable();
			readMessage();
			bDone = true;
		}

		qDebug() << "After ReadyRead byte Available: " << m_tcpSocket->bytesAvailable();

	} while (!bDone);
	
	return 0;
}

int SCBPrecizaTracker::getTransformMatrix(vtkMatrix4x4* output)
{
	bool bDone = false;

	if (!m_isConnected)  // not connected
	{
		qDebug() << "Tracker is not connected !\n";
		return 1;
	}

	if (!m_isTracking)  // not tracking
	{
		qDebug() << "Tracker is no tracking !\n";
		return 1;
	}

	this->sendMessage("req2");

	if (this->getRespond())
	{
		qDebug() << "No Respond! \n";
		return 2;
	}

	QString toolName = "vspTool2";
	int toolPos = m_szHandleInformation.indexOf(toolName);
	if (toolPos == -1)
	{
		qDebug() << "Tool is not found !\n";
		return 3;
	}

	QString xtag = "x";
	QString ytag = "y";
	QString ztag = "z";
	QString rtag = "rms";
	QString q0tag = "q0";
	QString q1tag = "q1";
	QString q2tag = "q2";
	QString q3tag = "q3";
	QString etag = "vep";
	int xpos = m_szHandleInformation.indexOf(xtag);
	QString ss_from_x;
	ss_from_x = m_szHandleInformation.mid(xpos);
	
	int ypos = ss_from_x.indexOf(ytag);
	int zpos = ss_from_x.indexOf(ztag);
	int rpos = ss_from_x.indexOf(rtag);
	int q0pos = ss_from_x.indexOf(q0tag);
	int q1pos = ss_from_x.indexOf(q1tag);
	int q2pos = ss_from_x.indexOf(q2tag);
	int q3pos = ss_from_x.indexOf(q3tag);
	int epos = ss_from_x.indexOf(etag);
	//  +1: to eliminate "x";
	QString xcoordinate = ss_from_x.mid(1, ypos - 1);
	QString ycoordinate = ss_from_x.mid(ypos + 1, zpos - (ypos + 1));
	QString zcoordinate = ss_from_x.mid(zpos + 1, rpos - (zpos + 1));
	QString rms_error = ss_from_x.mid(rpos + 3, q0pos - (rpos + 3));
	QString q0 = ss_from_x.mid(q0pos + 2, q1pos - (q0pos + 2));
	QString q1 = ss_from_x.mid(q1pos + 2, q2pos - (q1pos + 2));
	QString q2 = ss_from_x.mid(q2pos + 2, q3pos - (q2pos + 2));
	QString q3 = ss_from_x.mid(q3pos + 2, epos - (q3pos + 2));
	
	//show double, NOT string
	double val_x = xcoordinate.toDouble();
	double val_y = ycoordinate.toDouble();
	double val_z = zcoordinate.toDouble();
	double val_rms = rms_error.toDouble();
	double val_q0 = q0.toDouble();
	double val_q1 = q1.toDouble();
	double val_q2 = q2.toDouble();
	double val_q3 = q3.toDouble();
	qDebug("\n x: %f  y: %f  z: %f  rms: %f  q0: %f  q1: %f  q2: %f  q3: %f", val_x, val_y, val_z, val_rms, val_q0, val_q1, val_q2, val_q3);
	
	vtkSmartPointer<vtkMatrix4x4> tempMat = vtkSmartPointer<vtkMatrix4x4>::New();
	double q[4] = { val_q1, val_q2, val_q3, val_q0 };
	this->convertQuatToRotationMatrix(q, tempMat);
	tempMat->SetElement(0, 3, val_x);
	tempMat->SetElement(1, 3, val_y);
	tempMat->SetElement(2, 3, val_z);
	tempMat->Modified();
	
	output->DeepCopy(tempMat);
	return 0;
}

int SCBPrecizaTracker::getTransformMatrix(double* position, double* quaterion)
{
	if (!m_isConnected)  // not connected
	{
		qDebug() << "Tracker is not connected !\n";
		return 1;
	}

	if (!m_isTracking)  // not tracking
	{
		qDebug() << "Tracker is no tracking !\n";
		return 1;
	}

	this->sendMessage("req2");

	if (this->getRespond())
	{
		qDebug() << m_tcpSocket->peerAddress();
		qDebug() << m_tcpSocket->errorString();
		qDebug() << "No Respond! \n";
		return 2;
	}
	
	QString toolName = "vspTool2";
	int toolPos = m_szHandleInformation.indexOf(toolName);
	if (toolPos == -1)
	{
		qDebug() << "Tool is not found !\n";
		return 3;
	}

	QString xtag = "x";
	QString ytag = "y";
	QString ztag = "z";
	QString rtag = "rms";
	QString q0tag = "q0";
	QString q1tag = "q1";
	QString q2tag = "q2";
	QString q3tag = "q3";
	QString etag = "vep";
	int xpos = m_szHandleInformation.indexOf(xtag);
	QString ss_from_x;
	ss_from_x = m_szHandleInformation.mid(xpos);

	int ypos = ss_from_x.indexOf(ytag);
	int zpos = ss_from_x.indexOf(ztag);
	int rpos = ss_from_x.indexOf(rtag);
	int q0pos = ss_from_x.indexOf(q0tag);
	int q1pos = ss_from_x.indexOf(q1tag);
	int q2pos = ss_from_x.indexOf(q2tag);
	int q3pos = ss_from_x.indexOf(q3tag);
	int epos = ss_from_x.indexOf(etag);
	//  +1: to eliminate "x";
	QString xcoordinate = ss_from_x.mid(1, ypos - 1);
	QString ycoordinate = ss_from_x.mid(ypos + 1, zpos - (ypos + 1));
	QString zcoordinate = ss_from_x.mid(zpos + 1, rpos - (zpos + 1));
	QString rms_error = ss_from_x.mid(rpos + 3, q0pos - (rpos + 3));
	QString q0 = ss_from_x.mid(q0pos + 2, q1pos - (q0pos + 2));
	QString q1 = ss_from_x.mid(q1pos + 2, q2pos - (q1pos + 2));
	QString q2 = ss_from_x.mid(q2pos + 2, q3pos - (q2pos + 2));
	QString q3 = ss_from_x.mid(q3pos + 2, epos - (q3pos + 2));

	//show double, NOT string
	double val_x = xcoordinate.toDouble();
	double val_y = ycoordinate.toDouble();
	double val_z = zcoordinate.toDouble();
	double val_rms = rms_error.toDouble();
	double val_q0 = q0.toDouble();
	double val_q1 = q1.toDouble();
	double val_q2 = q2.toDouble();
	double val_q3 = q3.toDouble();
	qDebug("\n x: %f  y: %f  z: %f  rms: %f  q0: %f  q1: %f  q2: %f  q3: %f", val_x, val_y, val_z, val_rms, val_q0, val_q1, val_q2, val_q3);

	position[0] = val_x;
	position[1] = val_y;
	position[2] = val_z;

	quaterion[0] = val_q0;
	quaterion[1] = val_q1;
	quaterion[2] = val_q2;
	quaterion[3] = val_q3;

	return 0;
}

int SCBPrecizaTracker::getStrayMarkers(vtkPoints* point)
{
	vtkSmartPointer<vtkPoints> tempPoints = vtkSmartPointer<vtkPoints>::New();

	if (!m_isConnected)  // not connected
	{
		qDebug() << "Tracker is not connected !\n";
		return 1;
	}

	if (!m_isTracking)  // not tracking
	{
		qDebug() << "Tracker is no tracking !\n";
		return 1;
	}

	this->sendMessage("reqsm");

	if (this->getRespond())
	{
		qDebug() << "No Respond! \n";
		return 2;
	}

	QString pointStart = "vspMarkersCNT";

	int startPos = m_szHandleInformation.indexOf(pointStart);

	if (startPos == -1)
	{
		qDebug() << "No point found !\n";
		return 3;
	}
	int pos = m_szHandleInformation.indexOf("m1x");
	int numberOfPoints = m_szHandleInformation.mid(13, pos - 13).toInt();

	QString remainMessage;
	remainMessage = m_szHandleInformation;

	for (int i = 1; i < numberOfPoints + 1; i++)
	{
		QString numbertag = QString::number(i);
		QString xtag = numbertag + QString("x");
		QString ytag = numbertag + QString("y");
		QString ztag = numbertag + QString("z");
		QString endtag;
		int xpos = remainMessage.indexOf(xtag);
		remainMessage = remainMessage.mid(xpos);
		int ypos = remainMessage.indexOf(ytag);
		int zpos = remainMessage.indexOf(ztag);
		if (i == numberOfPoints)
		{
			endtag = "vep";
		}
		else
		{
			endtag = QString::number(i + 1).append("x");
		}
		int epos = remainMessage.indexOf(endtag);

		QString xcoordinate = remainMessage.mid(2, ypos - 3);
		QString ycoordinate = remainMessage.mid(ypos + 2, zpos - (ypos + 3));
		QString zcoordinate = "";
		if (i == numberOfPoints)
		{
			zcoordinate = remainMessage.mid(zpos + 2, epos - (zpos + 2));
		}
		else
		{
			zcoordinate = remainMessage.mid(zpos + 2, epos - (zpos + 3));
		}

		
		double tempPoint[3] = { xcoordinate.toDouble(), ycoordinate.toDouble(), zcoordinate.toDouble() };
		tempPoints->InsertNextPoint(tempPoint);
	}

	point->DeepCopy(tempPoints);

	return 0;
}
//int SCBPrecizaTracker::getTransformMatrix(vtkMatrix4x4* output)
//{
//	if (!m_isTracking)  // not tracking
//	{
//		qDebug() << "Tracker is no tracking !\n";
//		return 1;
//	}
//	QString toolName = "vspTool2";
//	int toolPos = m_szHandleInformation.indexOf(toolName);
//	if (toolPos == -1)
//	{
//		qDebug() << "Tool is not found !\n";
//		return 2;
//	}
//
//	QString xtag = "x";
//	QString ytag = "y";
//	QString ztag = "z";
//	QString rtag = "rms";
//	QString q0tag = "q0";
//	QString q1tag = "q1";
//	QString q2tag = "q2";
//	QString q3tag = "q3";
//	QString etag = "vep";
//	int xpos = m_szHandleInformation.indexOf(xtag);
//	QString ss_from_x;
//	ss_from_x = m_szHandleInformation.mid(xpos);
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
//	//show double, NOT string
//	double val_x = xcoordinate.toDouble();
//	double val_y = ycoordinate.toDouble();
//	double val_z = zcoordinate.toDouble();
//	double val_rms = rms_error.toDouble();
//	double val_q0 = q0.toDouble();
//	double val_q1 = q1.toDouble();
//	double val_q2 = q2.toDouble();
//	double val_q3 = q3.toDouble();
//	qDebug("\n x: %f  y: %f  z: %f  rms: %f  q0: %f  q1: %f  q2: %f  q3: %f", val_x, val_y, val_z, val_rms, val_q0, val_q1, val_q2, val_q3);
//
//	vtkSmartPointer<vtkMatrix4x4> tempMat = vtkSmartPointer<vtkMatrix4x4>::New();
//	double q[4] = { val_q1, val_q2, val_q3, val_q0 };
//	this->convertQuatToRotationMatrix(q, tempMat);
//	tempMat->SetElement(0, 3, val_x);
//	tempMat->SetElement(1, 3, val_y);
//	tempMat->SetElement(2, 3, val_z);
//	tempMat->Modified();
//
//	output->DeepCopy(tempMat);
//	return 0;
//}

//int SCBPrecizaTracker::getTransformMatrix(double* position, double* quaterion)
//{
//	if (!m_isTracking)  // not tracking
//	{
//		qDebug() << "Tracker is no tracking !\n";
//		return 1;
//	}
//	QString toolName = "vspTool2";
//	int toolPos = m_szHandleInformation.indexOf(toolName);
//	if (toolPos == -1)
//	{
//		qDebug() << "Tool is not found !\n";
//		return 2;
//	}
//
//	QString xtag = "x";
//	QString ytag = "y";
//	QString ztag = "z";
//	QString rtag = "rms";
//	QString q0tag = "q0";
//	QString q1tag = "q1";
//	QString q2tag = "q2";
//	QString q3tag = "q3";
//	QString etag = "vep";
//	int xpos = m_szHandleInformation.indexOf(xtag);
//	QString ss_from_x;
//	ss_from_x = m_szHandleInformation.mid(xpos);
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
//	//show double, NOT string
//	double val_x = xcoordinate.toDouble();
//	double val_y = ycoordinate.toDouble();
//	double val_z = zcoordinate.toDouble();
//	double val_rms = rms_error.toDouble();
//	double val_q0 = q0.toDouble();
//	double val_q1 = q1.toDouble();
//	double val_q2 = q2.toDouble();
//	double val_q3 = q3.toDouble();
//	qDebug("\n x: %f  y: %f  z: %f  rms: %f  q0: %f  q1: %f  q2: %f  q3: %f", val_x, val_y, val_z, val_rms, val_q0, val_q1, val_q2, val_q3);
//	
//	position[0] = val_x;
//	position[1] = val_y;
//	position[2] = val_z;
//
//	quaterion[0] = val_q0;
//	quaterion[1] = val_q1;
//	quaterion[2] = val_q2;
//	quaterion[3] = val_q3;
//
//	return 0;
//}


void SCBPrecizaTracker::startTracking()
{
	if (m_isConnected == false)
	{
		return;
	}
	//CMD: "start" (string type)  to VISTEKO Tracker to get started.
	QString strMesg = "start";
	qDebug() << "\n Start Tracking ...";
	m_tcpSocket->write(strMesg.toStdString().c_str(), strlen(strMesg.toStdString().c_str())); 
	m_isTracking = true;
}

void SCBPrecizaTracker::stopTracking() //·¢ËÍÍ£Ö¹¶¨Î»×·×ÙÃüÁî
{
	if (m_isConnected == false)
	{
		return;
	}
	//CMD: "stop" (string type)   to VISTEKO Tracker to get stopped.
	QString strMesg = "stop";
	qDebug() << "\n Stop Tracking ...";
	m_tcpSocket->write(strMesg.toStdString().c_str(), strlen(strMesg.toStdString().c_str()));
	m_isTracking = false;
}

/****** MATH FUNCTIONS FOR EULER AND QUATERNION MATH ******/
/********************************************************************
Name:        CvtQuatToRotationMatrix

Input Values:
QuatRotation
*pdtQuatRot :Ptr to the quaternion rotation.

Output Values:
RotationMatrix
dtRotationMatrix :The 3x3 determined rotation matrix.

Returned Value:
None.
Description:
This routine determines the rotation matrix that corresponds
to the given quaternion.

Let the quaternion be represented by:

	| Qx |
Q = | Qy |
	| Qz |
	| Q0 |

and the rotation matrix by:

	| M00 M01 M02 |
M = | M10 M11 M12 |
	| M20 M21 M22 |

then assuming the quaternion, Q, has been normalized to convert
Q to M we use the following equations:

M00 = (Q0 * Q0) + (Qx * Qx) - (Qy * Qy) - (Qz * Qz)
M01 = 2 * ((Qx * Qy) - (Q0 * Qz))
M02 = 2 * ((Qx * Qz) + (Q0 * Qy))
M10 = 2 * ((Qx * Qy) + (Q0 * Qz))
M11 = (Q0 * Q0) - (Qx * Qx) + (Qy * Qy) - (Qz * Qz)
M12 = 2 * ((Qy * Qz) - (Q0 * Qx))
M20 = 2 * ((Qx * Qz) - (Q0 * Qy))
M21 = 2 * ((Qy * Qz) + (Q0 * Qx))
M22 = (Q0 * Q0) - (Qx * Qx) - (Qy * Qy) + (Qz * Qz)
*********************************************************************/
void SCBPrecizaTracker::convertQuatToRotationMatrix(double* q, vtkMatrix4x4* matrix)
{
	double
		fQ0Q0,
		fQxQx,
		fQyQy,
		fQzQz,
		fQ0Qx,
		fQ0Qy,
		fQ0Qz,
		fQxQy,
		fQxQz,
		fQyQz;

	vtkSmartPointer<vtkMatrix4x4> vtkmatrix = vtkSmartPointer<vtkMatrix4x4>::New();
	vtkmatrix->Identity();
	/*
	* Determine some calculations done more than once.
	*/
	fQ0Q0 = q[3] * q[3];
	fQxQx = q[0] * q[0];
	fQyQy = q[1] * q[1];
	fQzQz = q[2] * q[2];
	fQ0Qx = q[3] * q[0];
	fQ0Qy = q[3] * q[1];
	fQ0Qz = q[3] * q[2];
	fQxQy = q[0] * q[1];
	fQxQz = q[0] * q[2];
	fQyQz = q[1] * q[2];

	/*
	* Determine the rotation matrix elements.
	*/
	vtkmatrix->SetElement(0, 0, (fQ0Q0 + fQxQx - fQyQy - fQzQz));
	vtkmatrix->SetElement(0, 1, (2.0 * (-fQ0Qz + fQxQy)));
	vtkmatrix->SetElement(0, 2, (2.0 * (fQ0Qy + fQxQz)));
	vtkmatrix->SetElement(1, 0, (2.0 * (fQ0Qz + fQxQy)));
	vtkmatrix->SetElement(1, 1, (fQ0Q0 - fQxQx + fQyQy - fQzQz));
	vtkmatrix->SetElement(1, 2, (2.0 * (-fQ0Qx + fQyQz)));
	vtkmatrix->SetElement(2, 0, (2.0 * (-fQ0Qy + fQxQz)));
	vtkmatrix->SetElement(2, 1, (2.0 * (fQ0Qx + fQyQz)));
	vtkmatrix->SetElement(2, 2, (fQ0Q0 - fQxQx - fQyQy + fQzQz));

	vtkmatrix->Modified();
	matrix->DeepCopy(vtkmatrix);
}

