import os
import json
from jinja2 import Template, FileSystemLoader, Environment
from yaml import parse


def Render(dict, tpl):
    j2_loader = FileSystemLoader(searchpath=os.path.dirname(tpl))
    env = Environment(loader=j2_loader)
    j2_tmpl = env.get_template(os.path.basename(tpl))
    return j2_tmpl.render(dict)