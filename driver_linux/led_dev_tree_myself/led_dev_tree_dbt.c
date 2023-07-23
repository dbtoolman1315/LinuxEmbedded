#define GROUP_PIN(x,y) ((x << 16) | (y))

myself_led@0
{
	compatible = "myself,led";
	pin = <GROUP_PIN(5,3)>;
};


