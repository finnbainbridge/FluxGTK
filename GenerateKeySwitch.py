import copy
array = list("abcdefghijklmnopqrstuvwxyz")
narray = copy.deepcopy(array)
for i in array:
    narray.append(i.upper())

array = narray

for i in range(25):
    array.append("F"+str(i+1))

for i in range(10):
    array.append(str(i))

# Manual ones
array.append("space")
array.append("apostrophe")
array.append("comma")
array.append("minus")
array.append("period")
array.append("slash")
array.append("semicolon")
array.append("equal")

array.append("Escape")
array.append("Return")
array.append("Tab")
array.append("BackSpace")
array.append("Insert")
array.append("Delete")
array.append("Right")
array.append("Left")
array.append("Down")
array.append("Up")
array.append("Page_Up")
array.append("Page_Down")
array.append("Home")
array.append("End")
array.append("Caps_Lock")
array.append("Scroll_Lock")
array.append("Num_Lock")
array.append("Print_Screen")
array.append("Pause")

for i in range(10):
    array.append("KP_" + str(i))

array.append("KP_Decimal")
array.append("KP_Divide")
array.append("KP_Multiply")
array.append("KP_Subtract")
array.append("KP_Add")
array.append("KP_Enter")
array.append("KP_Equal")
array.append("Menu")


for i in array:
    print("        case (GDK_KEY_%s): return FLUX_KEY_%s;" % (i, i.upper()))