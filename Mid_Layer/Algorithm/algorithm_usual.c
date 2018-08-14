#include <stdint.h>
#include "platform_common.h"
#include "algorithm_usual.h"


#define expsmasklen  4




//caculate the decima part of exp
static const float exps[4] =
{
2.71828182845904523536028747135, // exp(1)
7.38905609893065022723042746058, // exp(2)
54.5981500331442390781102612029, // exp(4)
2980.95798704172827474359209945 // exp(8)
};

static const unsigned char expsmask[4] = {1, 2, 4, 8};
static const float Exp_Cal_Boundary = 0.0001;//控制exp算法计算次数
static const float ln10 = 2.30258;//exchange ln(x) to lg(x)



//**********************************************************************
/**
 * @brief      使用泰勒展开计算自然对数lnx
 * @param[in]  x   输入
 * @return     结果
 */
float Algorithm_ln(float x)
{
	//算法1
	float return_temp = 0;
	unsigned char index = 0, k = 0;
	float i, y, y2, taylor = 0, yn = 1;
	
	if(x <= 0)
		return return_temp;
	
	while (1) 
	{
		if (x > (float)0.5)
		{
			x /= 2;
			k++;
		} 
		else 
		{
			break;
		}
	}
	
	y = (x - 1) / (x + 1);

	y2 = y * y;
	do
	{
		i = 1.0 / (2 * (index++) + 1) * yn;
		taylor += i;
		yn *= y2;
//		if(index >= 255)
//			break;
	} while (i > (float)0.0001);

	return_temp = (2 * y * taylor + k * 0.693147);

	return return_temp;
	
	
	//算法2
//		static float ln10 = 2.30258;
//		static float lnr = 0.20024;
//		unsigned  short i = 3;
//		int k =0,l =0;
//		float v,y,y2,t,z;
//		
//		for(;x>1;k++)	x/=10.0;
//		for(;x<=0.1;k--) x*=10.0;
//		for(;x<0.904;l--) x*=1.221;
//		
//		y = (x - 1) / (x + 1);
//		v = 1;
//		y2 = y * y;
//		t = y2;
//		z = t / 3;
//		
//		for(i = 3;z !=0;z =(t *=y2)/(i +=2)) v+=z;
//		
//		return_temp =(k * ln10+l * lnr + 2 * y * v)/ln10;
//		if(x_temp>1&&x_temp<=1.4)
//			return_temp +=0.00281;
//		if(x_temp>1.4)
//			return_temp +=0.002;
//		return return_temp;//lg
}

//**********************************************************************
/**
 * @brief      使用泰勒展开计算对数lgx
 * @param[in]  x   输入
 * @return     结果
 */
float Algorithm_lg(float x)
{
	float x_temp = x;

	x_temp = Algorithm_ln(x);

	return x_temp / ln10;
}

//**********************************************************************
/**
 * @brief      使用float型数据四舍五入，转换成整数
 * @param[in]  x   输入， 应当在int 型范围内
 * @return     结果
 */
int Algorithm_round(float x)
{
	int x_temp;

	if (x >= 0)
	{
		x_temp  = (int)(x * 10.0f);
		if (x_temp % 10 >= 5)
		{
			x_temp += 10;
		}
		x_temp = x_temp / 10;
	}
	else
	{
		x_temp = (int) (x * (-10.0));
		if (x_temp % 10 >= 5)
		{
			x_temp += 10;
		}
		x_temp = (-1) * (x_temp / 10);
	}
	return x_temp;
}

//**********************************************************************
/**
 * @brief      float型数据取绝对值
 * @param[in]  x   输入
 * @return     结果
 */
float Algorithm_abs(float x)
{
	float x_temp = x;

//	if (x >= 0)
//	{
//		x_temp = x;
//	}
//	else
	if (x < 0)
	{
		x_temp = (-1) * x;
	}
	return x_temp;
}

//**********************************************************************
/**
 * @brief      使用泰勒展开计算指数exp(x)
 * @param[in]  x   输入
 * @return     结果
 */
 float Algorithm_exponential(float q)
{ // q (almost) in [ -0.5, 0.5 ]
	float y = 1, t = 1;
	unsigned char i;
	
	for ( i = 1; Algorithm_abs(t) > Exp_Cal_Boundary; i++)
	{
		t *= q / i;
		y += t;		
	} 
	if(q > 0)//compensatiton for err
	{
		y = y - ((0.0099f) * y);
	}
	return y;
}

//**********************************************************************
/**
 * @brief      使用泰勒展开计算指数exp(x)
 * @param[in]  x   输入
 * @return     结果
 */
float Algorithm_exp(float x)
{
	int n;
	unsigned int m;
	float z = 1;
	float x_temp = x;
	float y;
	unsigned char i;

	n = Algorithm_round(x_temp);

	//小数部分计算，泰勒展开
	y = Algorithm_exponential(x_temp - n);

	m = (n < 0)? ((-1)* n):n;
	//整数部分计算，取常量值
	for (i = 0; i < expsmasklen; i++)
	{
		if (m & expsmask[i])
		{
			z *= exps[i];
		}
	}
	if(x > 0)
	{
		y = y - ((0.0099f) * y);
	}
	return (n <0)?(y / z):(y * z);
}

//**********************************************************************
/**
 * @brief      三角函数sin(x)泰勒展开:
 //sinx= x- x^3/3! + x^5/5! - x^7/7!+ x^9/9! .... =∑(-1)^n x^(2n+1)/(2n+1)!
 * @param[in]  x   输入弧度值
 * @return     结果
 */
float Algorithm_sin(float x)
{
	int n =1, sign = -1;
	float fz = x, sum = x, fs;
	float fm = 1;
	float x2 = x * x;	
	do  
	{  
		fz		= fz * x2;  
		fm 		= fm * (n+1) * (n+2); 			
		fs 		= fz / fm * sign;  
		sign 	= -sign;  
		sum 	+= fs;  
		n 		+= 2;  
	}while(Algorithm_abs(fs)>0.000001f);

	return sum;
}

//**********************************************************************
/**
 * @brief    三角函数cos(x)泰勒展开
 * @param[in]  x 输入弧度值
 * @return     结果
 */
float Algorithm_cos(float x)
{
	int n=0,sign=-1;  
    float fz=1,sum=1,fs; 
	float  fm=1;	
    double x2 = x * x;
	
	do  
	{  
		fz 		= fz * x2;  
		fm 		= fm * (n+1) * (n+2);  
		fs 		= fz / (sign*fm);  
		sign 	= -sign;  
		sum 	+= fs;  
		n 		+= 2;  
	}while(Algorithm_abs(fs)>0.0000001f);

    return sum;
}

//**********************************************************************
/**
 * @brief     三角函数tan(x)泰勒展开
 * @param[in]  x   输入
 * @return     结果
 */
float Algorithm_tan(float x)
{
	//方法一
	return Algorithm_sin(x)/Algorithm_cos(x);
	//方法二

}


//**********************************************************************
/**
 * @brief    三角函数arcsin(x)泰勒展开
 * @param[in]  x   输入
 * @return     结果
 */
float Algorithm_arcsin(float x)
{
	return 1;
}

//**********************************************************************
/**
 * @brief    三角函数arccos(x)泰勒展开
 //arccos x = π - ( x + 1/2*x^3/3 + 1*3/(2*4)*x^5/5 + …… ) (|x|<1) 
 * @param[in]  x   输入数值
 * @return     结果，弧度值
 */
float Algorithm_arccos(float x)
{
	int16 i;  
	
    float ans=x,t1=1,t2=x;
	float x2 = x * x;  

    if (Algorithm_abs(x) >= 1)
    {
    	return 1;
    }
	
    for(i = 3;i < 51;i +=2)
    {  
        t1 		*= (float)(i-2)/(float)(i-1);  
        t2 		*= x2;  
        ans 	+= (t1*t2/(float)i);  
    }  
    ans= ((float)PI  - ans)/2.0f;
    return ans;
}

//**********************************************************************
/**
 * @brief    三角函数arctan(x)泰勒展开
 * @param[in]  x   输入
 * @return     结果
 */
float Algorithm_arctan(float x)
{
	return 1;
}






