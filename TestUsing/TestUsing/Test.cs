﻿using CulverinEditor;
using CulverinEditor.Debug;

public class Tank_script
{

    public GameObject tank;

    void Start()
    {

    }

    void Update()
    {
        Debug.Log(tank.name);
        if (Input.KeyRepeat("UP_ARROW"))
        {
            tank.GetComponent<Transform>().position += Vector3.Forward;
        }
        else if (Input.KeyRepeat("DOWN_ARROW"))
        {
            tank.GetComponent<Transform>().position += Vector3.Backward;
        }
        else if (Input.KeyRepeat("LEFT_ARROW"))
        {
            tank.GetComponent<Transform>().position += Vector3.Left;
        }
        else if (Input.KeyRepeat("RIGHT_ARROW"))
        {
            tank.GetComponent<Transform>().position += Vector3.Right;
        }
    }
}