import pyjsmn

jsonstr = """\
{
    "name" : "Jack",
    "pc" : {"windows": "XP", "linux": null}
}"""
print(pyjsmn.loads(jsonstr))
