#include "TimeUnit.h"

void convert(TimeUnit a, TimeUnit b, uint32_t& nom, uint32_t& denom){
	nom=1;
	denom=1;
	if((a==TimeUnit::us && b==TimeUnit::ms) || (a==TimeUnit::ms && b==TimeUnit::s)){
		nom=1;
		denom=1000;
	}
	if((a==TimeUnit::ms && b==TimeUnit::us) || (a==TimeUnit::s && b==TimeUnit::ms)){
		nom=1000;
		denom=1;
	}
	if(a==TimeUnit::us && b==TimeUnit::s){
		nom=1;
		denom=1000000;
	}
	if(a==TimeUnit::s && b==TimeUnit::us){
		nom=1000000;
		denom=1;
	}	
}
