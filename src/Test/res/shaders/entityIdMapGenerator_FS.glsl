#version 330

flat in uint v_entityId;

out uint o_entityId;

void main()
{
	o_entityId = v_entityId;
}