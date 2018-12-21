#ifndef MINIPID_H
#define MINIPID_H

class IIRFilter;

template <size_t N>
class Cascade;

class MiniPID{
public:
	MiniPID(double, double, double);
	MiniPID(double, double, double, double);
	void setP(double);
	void setI(double);
	void setD(double);
	void setF(double);
	void setPID(double, double, double);
	void setPID(double, double, double, double);
	void setMaxIOutput(double);
	void setOutputLimits(double);
	void setOutputLimits(double,double);
	void setDirection(bool);
	void setSetpoint(double);
	void reset();
	void setOutputRampRate(double);
	void setSetpointRange(double);
	void setOutputFilter(double);
	double getOutput();
	double getOutput(double);
	double getOutput(double, double);

    double GetP() const {return lastP;}
    double GetI() const {return lastI;}
    double GetD() const {return lastD;}

private:
	double clamp(double, double, double);
	bool bounded(double, double, double);
	void checkSigns();
	void init();
	double P;
	double I;
	double D;
	double F;

	double maxIOutput;
	double maxError;
	double errorSum;

	double maxOutput; 
	double minOutput;

	double setpoint;

	double lastActual;

	bool firstRun;
	bool reversed;

	double outputRampRate;
	double lastOutput;

	double outputFilter;

	double setpointRange;


	IIRFilter* derivativeFilter0 = nullptr;
	IIRFilter* derivativeFilter1 = nullptr;
	IIRFilter* derivativeFilter2 = nullptr;
	Cascade<3> *derivativeCascade = nullptr;
	double lastP = 0.0;
	double lastI = 0.0;
	double lastD = 0.0;
};
#endif
