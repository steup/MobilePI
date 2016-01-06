#include <avr/io.h>
#include <util/delay.h>

class Pin{
	private:
		static const int8_t ddrOffset = -1;
		static const int8_t pinOffset = -2;
		volatile uint8_t& mPORT;
		volatile uint8_t& mDDR;
		volatile uint8_t& mPIN;
		const uint8_t mByte;
	protected:
		void setDDR(bool out) { 
			if(out)
				mDDR |= mByte; 
			else
				mDDR &= ~mByte; 
		}
		void setPORT(bool value) { 
			if(value)
				mPORT |= mByte; 
			else
				mPORT &= ~mByte; 
		}
		void toggle() { 
			mPIN = mByte; 
		}
		bool getPIN() {
			return mPIN & mByte;
		}
		bool getPORT() {
			return mPORT & mByte;
		}


	public:
		Pin(volatile uint8_t& port, uint8_t pin)
			:	mPORT(port), mDDR(*(&port+ddrOffset)), mPIN(*(&port+pinOffset)), mByte(1<<pin) {
		}

		~Pin() {
			setDDR(false);
			setPORT(false);
		}
};

class PinIn : public Pin {
	private:
		bool mInvert;
	public:
		PinIn(volatile uint8_t& port, uint8_t pin, bool pullup, bool invert=false)
			: Pin(port, pin), mInvert(invert) {
			setDDR(false);
			setPORT(pullup);
		}
		bool read() {
			if(mInvert)
				return !getPIN();
			else
				return getPIN();
		}
};

class PinOut : public Pin {
	private:
		bool mInvert;
	public:
		PinOut(volatile uint8_t& port, uint8_t pin, bool value, bool invert=false)
		: Pin(port, pin), mInvert(invert) {
			setDDR(true);
			set(value);
		}

		void set(bool value) {
			if(mInvert)
				setPORT(!value);
			else
				setPORT(value);
		}

		void on() {
			set(true);
		}

		void off() {
			set(false);
		}

		bool state() {
			if(mInvert)
				return !getPORT();
			else
				return getPORT();
		}

		using Pin::toggle;
};

int main(){
	PinOut statusLed(PORTB, 7, false, true);
	PinOut oc0(PORTB, 4, false);
	PinOut oc1a(PORTB, 5, false);
	PinOut oc1b(PORTB, 6, false);
	PinOut miso(PORTB, 3, false);
	PinIn  int5(PORTE, 5, true);
	PinIn  int6(PORTE, 6, true);
	PinIn  int7(PORTE, 7, true);
	PinIn  ad0(PORTF, 2, true);
	PinIn  ad1(PORTF, 3, true);
	while(1){
		_delay_ms(10);
		statusLed.toggle();
		miso.toggle();
		oc0.toggle();
		oc1a.toggle();
		oc1b.toggle();
		if(!int5.read() || !int6.read() || !int7.read() || !ad0.read() || !ad1.read())
			statusLed.on();
	}
	return 0;
}
