
from xml.dom import minidom
import re

IMAGE_DIR = '.'
STYLESHEET = 'houdini.css'

RE_ATTR_NAME = re.compile(r'(\w+).*\((\w+)\)')
RE_TOKEN = re.compile(r'\w+')
RE_SIMPLESECT = re.compile(r'<simplesect kind="(par|see)"[^>]*>.*?</simplesect>', re.MULTILINE)
RE_XML_TAGS = re.compile(r'<(simplesect|\w+description)( \w+=".*?")*>|</(simplesect|\w+description)>')
ATTRIBUTE_FLAGS = ['storable', 'readable', 'writable', 'connectable', 'keyable', 'hidden', 'array']
XML_HTML_MAP = {'title' : 'h2', 'para' : 'p'}

PAGE_HTML_TEMPLATE = (
"""<?xml version="1.0"?>
<html>
  <head>
    <title>%(name)s node</title>
    <meta name="section" content="Nodes"/>
    <meta name="module" content="MEL and expressions"/>
    <link rel="StyleSheet" href="%(stylesheet)s" type="text/css" media="all"/>
  </head>
  <body class="node">
  %(body)s
  </body>
</html>
"""
)

NODE_HTML_TEMPLATE = (
"""<div id="banner">
  <h1>%(name)s</h1>
</div>
<p>%(description)s</p>
""")

ATTRIBUTE_HTML_TEMPLATE = (
"""<a name="hAttr"/>
<h2>Attributes (%(attributes)i)</h2>
<p>
%(attribute_link_html)s
</p>
<table border="0" width="100%%">
  <tr>
    <th bgcolor="#CCCCCC" width="50%%">Long name (short name)</th>
    <th bgcolor="#CCCCCC" width="10%%">Type</th>
    <th bgcolor="#CCCCCC" width="20%%">Default</th>
    <th bgcolor="#CCCCCC" width="20%%">
      <a href="flagHelp.html">Flags</a>
    </th>
  </tr>
  %(attribute_html)s
</table>
"""
)

ATTRIBUTE_LINK_HTML_TEMPLATE = """<a href="#attr%(long_name)s">%(long_name)s</a>"""

ATTRIBUTE_FLAG_HTML_TEMPLATE = (
"""<img src="%(image_dir)s/%(flag)s.gif" alt="%(flag)s" title="%(flag)s" border="0" width="13" height="13" hspace="2"/>""")

ATTRIBUTE_TABLE_HTML_TEMPLATE = (
"""
<tr>
  <td colspan="4"/>
</tr>
<tr bgcolor="#EEEEEE">
  <td class="attrName" width="50%%" valign="top"><a name="attr%(long_name)s"/><b><code>%(long_name)s</code></b>
    (<b><code>%(short_name)s</code></b>)
  </td>
  <td class="attrType" width="10%%" valign="top">%(type)s</td>
  <td class="attrType" width="20%%" valign="top">%(default)s</td>
  <td class="attrFlags" width="20%%" valign="top">
  %(flag_html)s
  </td>
</tr>
<tr>
  <td class="attrComment" colspan="4">
    <table width="100%%">
  	<tr>
  	  <td width="5%%"/>
  	  <td>%(description)s</td>
  	</tr>
    </table>
  </td>
</tr>
""")

CHILD_ATTRIBUTE_HTML_TEMPLATE = (
"""
<tr>
  <td colspan="4" align="right">
    <table width="95%%">
    %(child_attribute_html)s
    </table>
  </td>
</tr>
""")

ENUM_HTML_TEMPLATE = '<tr><td valign="top">%(value)i</td><td valign="top">%(name)s</td><td valign="top">%(description)s</td>'

def warning(string):
	print string

def doxygen_id(name):
	result = ''
	for letter in name:
		if letter.isupper(): result += "_%s" % letter.lower()
		else: result += letter
	return result

def doxygen_class_id(class_name):
	return 'class' + doxygen_id(class_name)

def get_tag_value(element, tag):
	elements = element.getElementsByTagName(tag)
	if not elements: return None
	return elements[0].firstChild.nodeValue

def find_tag(elements, tag, value):
	for element in elements:
		if get_tag_value(element, tag) == value:
			return element
	return None

def get_doxygen_enum(doc, enum):
	element = find_tag(doc.getElementsByTagName('memberdef'), 'name', enum)
	if not element: return None
	enumvalues = element.getElementsByTagName('enumvalue')
	result = []
	for enumvalue in enumvalues:
		name = get_tag_value(enumvalue, 'name')
		value = int(get_tag_value(enumvalue, 'initializer') or -1)
		description = get_element_description(enumvalue)
		result.append({'name': name, 'value' : value, 'description' : description})
	return result

def get_element_description(element):

	brief = ''
	detailed = ''
	inbody = ''

	for child in element.childNodes:
		if child.localName == 'briefdescription': brief = child.toxml()
		if child.localName == 'detaileddescription': detailed = child.toxml()
		if child.localName == 'inbodydescription': inbody = child.toxml()

	return clean_description(brief + detailed + inbody)

def clean_description(description):

	description = RE_SIMPLESECT.sub('', description)
	description = RE_XML_TAGS.sub('', description)
	description = description.strip()
	description = '\n'.join([x for x in description.split('\n') if x.strip()])
	for search, replace in XML_HTML_MAP.items():
		description = description.replace('<%s>' % search, '<%s>' % replace)
		description = description.replace('</%s>' % search, '</%s>' % replace)
		description = description.replace('<%s/>' % search, '<%s/>' % replace)

	return description

class HoudiniAttribute:
	def __init__(self):
		self.long_name = ''
		self.short_name = ''
		self.description = ''
		self.type = ''
		self.default = ''
		self.storable = True
		self.readable = True
		self.writable = True
		self.connectable = True
		self.keyable = False
		self.hidden = False
		self.array = False
		self.children = []
		self.enum = None
	
	def get_flags(self):
		result = []
		for attr in ATTRIBUTE_FLAGS:
			if getattr(self, attr): result.append(attr)
		return result

	def __str__(self):
		result = '%s (%s)\n' % (self.long_name, self.short_name)
		result += '%s\n' % self.description
		result += 'Type: %s\n' % self.type
		result += 'Default: %s\n' % self.default
		result += 'Flags: %s\n' % ', '.join(self.get_flags())

		if self.enum:
			for enum in self.enum: result = "%(name) - %(value)i - %(description)s" % enum

		if len(self.children):
			result += 'Children:\n'
			for child in self.children:
				result += '\n'.join(['\t' + x for x in str(child).split('\n')]) + '\n'
		return result

	def html(self):

		flag_html = ''.join([ATTRIBUTE_FLAG_HTML_TEMPLATE % {'flag': flag, 'image_dir': IMAGE_DIR} for flag in self.get_flags()])

		description = self.description
		if self.enum:
			description += '<table>'
			for enum in self.enum: description += ENUM_HTML_TEMPLATE % enum
			description += '</table>'

		result = ATTRIBUTE_TABLE_HTML_TEMPLATE % {
					'long_name'   : self.long_name,
					'short_name'  : self.short_name,
					'type'        : self.type,
					'default'     : self.default,
					'flag_html'   : flag_html,
					'description' : description
				}

		if self.children:
			child_attribute_html = '\n'.join([x.html() for x in self.children])
			result += CHILD_ATTRIBUTE_HTML_TEMPLATE % {'child_attribute_html' : child_attribute_html }

		return result

class HoudiniNode:
	def __init__(self):
		self.name = ''
		self.class_name = ''
		self.description = ''
		self.attributes = {}
		self.top_level_attributes = []
	
	def __str__(self):
		result = '%s\n' % self.name
		result += '%s\n' % self.description
		result += 'Attributes:\n'
		for attribute in self.top_level_attributes:
			result += '\n'.join(['\t' + x for x in str(attribute).split('\n')]) + '\n'
		return result
	
	def attribute_html(self):
		attribute_link_html = ', '.join([ATTRIBUTE_LINK_HTML_TEMPLATE % {'long_name':name} for name in self.attributes.keys()])
		attribute_html = '\n'.join([attribute.html() for attribute in self.top_level_attributes])

		return ATTRIBUTE_HTML_TEMPLATE % {
					'attributes'          : len(self.attributes),
					'attribute_link_html' : attribute_link_html,
					'attribute_html'      : attribute_html,
				}

	def node_html(self):

		return NODE_HTML_TEMPLATE % {
					'name'                : self.name,
					'description'         : self.description,#.replace('\n', '<br/>'),
					'stylesheet'          : STYLESHEET
				}

	def html(self):

		return self.node_html() + self.attribute_html()


class MoxyDoc:

	def __init__(self, doxygen_xml_dir):
		self.doxygen_xml_dir = doxygen_xml_dir
		self.class_xml_docs = {}

	def load_class_xml(self, class_name):

		if not self.class_xml_docs.has_key(class_name):
			filename = "%s/%s.xml" % (self.doxygen_xml_dir, doxygen_class_id(class_name))
			self.class_xml_docs[class_name] = minidom.parse(filename)

		return self.class_xml_docs[class_name]

	def get_node(self, class_name, node_name):

		node = HoudiniNode()
		node.class_name = class_name
		node.name = node_name

		self.get_node_description(node)
		self.get_node_attributes(node)

		return node

	def write_node_html(self, clas_name, node_name, out):
		node = self.get_node(clas_name, node_name)
		out.write(node.html())

	def write_attribute_html(self, clas_name, node_name, out):
		node = self.get_node(clas_name, node_name)
		out.write(node.attribute_html())

	def get_node_description(self, node):

		doc = self.load_class_xml(node.class_name)

		[compounddef] = doc.getElementsByTagName('compounddef')

		node.description = get_element_description(compounddef)

	def get_node_attributes(self, node):

		doc = self.load_class_xml(node.class_name)

		members = doc.getElementsByTagName("memberdef")

		attributes = []

		for member in members:
			if member.getAttribute("kind") != 'variable': continue
			sections = member.getElementsByTagName("simplesect")
			if find_tag(sections, 'title', 'Houdini Attribute Name:'):
				attributes.append((member, sections))

		parents_children = []

		for (attribute, sections) in attributes:
			houdiniattr = HoudiniAttribute()

			tag = find_tag(sections, 'title', 'Houdini Attribute Name:')
			if tag:
				tag = tag.getElementsByTagName('para')[0]
				if tag.firstChild:
					name = tag.firstChild.nodeValue.strip()
					names = RE_ATTR_NAME.match(name)
					if names:
						houdiniattr.long_name = names.group(1)
						houdiniattr.short_name = names.group(2)
					else:
						warning('Name not specified for attribute '+get_tag_value(attribute, 'definition'))
						continue
			
			tag = find_tag(sections, 'title', 'Houdini Attribute Type:')
			if tag:
				tag = tag.getElementsByTagName('para')[0]
				if tag.firstChild:
					type = tag.firstChild.nodeValue.strip()
					if type.startswith('enum') and len(type.split()) > 1:
						houdiniattr.type = 'enum'
						houdiniattr.enum = get_doxygen_enum(doc, type.split()[1])
					else:
						houdiniattr.type = tag.firstChild.nodeValue.strip()

			tag = find_tag(sections, 'title', 'Houdini Attribute Default:')
			if tag:
				tag = tag.getElementsByTagName('para')[0]
				if tag.firstChild:
					houdiniattr.default = tag.firstChild.nodeValue.strip()

			tag = find_tag(sections, 'title', 'Houdini Attribute Flags:')
			if tag:
				tag = tag.getElementsByTagName('para')[0]
				flags = tag.firstChild.nodeValue.strip().split()
				for flag in flags:
					value = not flag.startswith('-')
					flag = RE_TOKEN.search(flag).group(0)
					if flag in ATTRIBUTE_FLAGS:
						setattr(houdiniattr, flag, value)

			tag = find_tag(sections, 'title', 'Houdini Attribute Parent:')
			if tag:
				tag = tag.getElementsByTagName('para')[0]
				if tag.firstChild:
					parents_children.append((tag.firstChild.nodeValue.strip(),houdiniattr.long_name))

			houdiniattr.description = get_element_description(attribute)

			node.attributes[houdiniattr.long_name] = houdiniattr
			node.top_level_attributes.append(houdiniattr)

		for parent, child in parents_children:
			try:
				node.attributes[parent].children.append(node.attributes[child])
				node.top_level_attributes.remove(node.attributes[child])
			except:
				warning('Error adding %s as a child of %s' % (child, parent))


if __name__ == '__main__':

	import sys

	try:
		xml, classname, nodename = sys.argv[1:4]
		output = sys.stdout
		if len(sys.argv) > 4: output = file(sys.argv[4], 'w')
	except:
		print "Usage: %s <doxygen xml dir> <C++ class name> <Houdini node name> [<output file name>]" % sys.argv[0]
		sys.exit(1)

	doc = MoxyDoc(xml)

	doc.write_attribute_html(classname, nodename, output)

