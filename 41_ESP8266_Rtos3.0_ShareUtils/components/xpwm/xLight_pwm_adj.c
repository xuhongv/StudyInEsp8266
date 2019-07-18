
#include "xLight_pwm.h"

#define ABS_MINUS(x, y) (x < y ? (y - x) : (x - y))

uint16_t min_ms = 2;
uint8_t light_sleep_flg = 0;

bool change_finish = true;

uint32_t current_duty[PWM_CHANNEL] = {0};
static uint32_t duty_now[PWM_CHANNEL] = {0};

extern struct light_saved_param light_param;

//-----------------------------------Light para storage---------------------------

struct pwm_param
{
	uint32_t period;			/**< PWM period */
	uint32_t freq;				/**< PWM frequency */
	uint32_t duty[PWM_CHANNEL]; /**< PWM duty */
};

static uint8_t CurFreeLightEvtIdx = 0;
static uint8_t TotalUsedLightEvtNum = 0;
static uint8_t CurEvtIdxToBeUse = 0;

#define LIGHT_EVT_QNUM (80)
static struct pwm_param LightEvtArr[LIGHT_EVT_QNUM];
//------------------------------------------------------------------------------------
static struct pwm_param *LightEvtMalloc(void)
{
	struct pwm_param *tmp = NULL;
	TotalUsedLightEvtNum++;
	if (TotalUsedLightEvtNum > LIGHT_EVT_QNUM)
	{
		TotalUsedLightEvtNum--;
	}
	else
	{
		tmp = &(LightEvtArr[CurFreeLightEvtIdx]);
		CurFreeLightEvtIdx++;
		if (CurFreeLightEvtIdx > (LIGHT_EVT_QNUM - 1))
			CurFreeLightEvtIdx = 0;
	}
	return tmp;
}

static void LightEvtFree(void)
{
	TotalUsedLightEvtNum--;
}

//------------------------------------------------------------------------------------

static void light_pwm_smooth_adj_proc(void);

void light_save_target_duty()
{
	//自行处理保存，无关紧要，不需要也可以！
}

void light_set_aim_r(uint32_t r)
{
	current_duty[2] = r;
	light_pwm_smooth_adj_proc();
}

void light_set_aim_g(uint32_t g)
{
	current_duty[3] = g;
	light_pwm_smooth_adj_proc();
}

void light_set_aim_b(uint32_t b)
{
	current_duty[4] = b;
	light_pwm_smooth_adj_proc();
}

void light_set_aim_cw(uint32_t cw)
{
	current_duty[0] = cw;
	light_pwm_smooth_adj_proc();
}

void light_set_aim_ww(uint32_t ww)
{
	current_duty[1] = ww;
	light_pwm_smooth_adj_proc();
}

static bool check_pwm_current_duty_diff()
{
	int i;

	for (i = 0; i < PWM_CHANNEL; i++)
	{
		if (light_param.pwm_duty[i] != current_duty[i])
		{
			return true;
		}
	}
	return false;
}

static void light_pwm_dh_adj_proc(void *Targ)
{
	uint8_t i;
	for (i = 0; i < PWM_CHANNEL; i++)
	{
		duty_now[i] = (duty_now[i] * 7 + current_duty[i]) >> 3;
		if (ABS_MINUS(duty_now[i], current_duty[i]) < 20)
		{
			duty_now[i] = current_duty[i];
		}
		user_light_set_duty(duty_now[i], i);
	}
	pwm_start();
	if (check_pwm_current_duty_diff())
	{
		change_finish = 0;
		light_pwm_dh_adj_proc(NULL);
	}
	else
	{
		change_finish = 1;
		light_pwm_smooth_adj_proc();
	}
}

static bool check_pwm_duty_zero()
{
	int i;
	for (i = 0; i < PWM_CHANNEL; i++)
	{
		if (light_param.pwm_duty[i] != 0)
		{
			return false;
		}
	}
	return true;
}

static void light_pwm_smooth_adj_proc(void)
{

	if (TotalUsedLightEvtNum > 0)
	{
		user_light_set_period(LightEvtArr[CurEvtIdxToBeUse].period);
		memcpy(current_duty, LightEvtArr[CurEvtIdxToBeUse].duty, sizeof(current_duty));
		CurEvtIdxToBeUse++;
		if (CurEvtIdxToBeUse > (LIGHT_EVT_QNUM - 1))
		{
			CurEvtIdxToBeUse = 0;
		}
		LightEvtFree();
		if (change_finish)
		{
			light_pwm_dh_adj_proc(NULL);
		}
	}

	if (change_finish)
	{
		light_save_target_duty();
		if (check_pwm_duty_zero())
		{
			if (light_sleep_flg == 0)
			{
				light_sleep_flg = 1;
			}
		}
	}
}

void light_set_aim_all(uint32_t cw, uint32_t ww, uint32_t r, uint32_t g, uint32_t b)
{
	struct pwm_param *tmp = LightEvtMalloc();
	if (tmp != NULL && !(cw > 100) && !(ww > 100) && !(r > 100) && !(g > 100) && !(g > 100))
	{

		tmp->period = PWM_PERIOD;
		tmp->duty[0] = cw * PWM_PERIOD / 100;
		tmp->duty[1] = ww * PWM_PERIOD / 100;
		tmp->duty[2] = r * PWM_PERIOD / 100;
		tmp->duty[3] = g * PWM_PERIOD / 100;
		tmp->duty[4] = b * PWM_PERIOD / 100;
		light_pwm_smooth_adj_proc();

		printf(" set cw[%d] ww[%d] r[%d] g[%d] b[%d] \n", tmp->duty[0], tmp->duty[1], tmp->duty[2], tmp->duty[3], tmp->duty[4]);
	}
}
