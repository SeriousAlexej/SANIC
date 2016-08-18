/*
build with the following command!
flexc++ -l objScanner.cpp -i objScanner.impl.h -c objScanner.h -b objScanner.base.h objScanner.l
*/
%class-name ObjScanner

%x comment mtllib usemtl object group

%%
g[ \t]+		{
			begin(StartCondition__::group);
		}
<group>{
\n	{
			begin(StartCondition__::INITIAL);
			return ObjParser::GROUP;
		}
.	more();
}
o[ \t]+		{
			begin(StartCondition__::object);
		}
<object>{
\n	{
			begin(StartCondition__::INITIAL);
			return ObjParser::OBJECT;
		}
	.	more();
}
mtllib[ \t]+	{
			begin(StartCondition__::mtllib);
		}
<mtllib>{
\n	{
			begin(StartCondition__::INITIAL);
			return ObjParser::MTLLIB;
		}
	.	more();
}
usemtl[ \t]+	{
			begin(StartCondition__::usemtl);
		}
<usemtl>{
\n	{
			begin(StartCondition__::INITIAL);
			return ObjParser::USEMTL;
		}
	.	more();
}
#		{
			begin(StartCondition__::comment);
		}
<comment>{
\n	{
			begin(StartCondition__::INITIAL);
		}
	.	more();
}
v		return ObjParser::VERTEX;
vt		return ObjParser::TEXCOORD;
vn		return ObjParser::NORMAL;
vp		return ObjParser::PARAMETER;
f		return ObjParser::POLYGON;
s		return ObjParser::SMOOTH;
off		return ObjParser::OFF;
\/		return '/';
\n		return '\n';
[-]?[0-9]+[.]?[0-9]* |
[-]?[0-9]+[.]?[0-9]*e[+-]?[0-9]*	return ObjParser::FLOAT;
[ \t]		/*ignore whitespaces*/;

