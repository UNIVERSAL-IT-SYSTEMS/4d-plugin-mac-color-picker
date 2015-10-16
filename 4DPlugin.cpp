/* --------------------------------------------------------------------------------
 #
 #	4DPlugin.cpp
 #	source generated by 4D Plugin Wizard
 #	Project : Mac Color Picker
 #	author : miyako
 #	2015/10/16
 #
 # --------------------------------------------------------------------------------*/


#include "4DPluginAPI.h"
#include "4DPlugin.h"

void PluginMain(PA_long32 selector, PA_PluginParameters params)
{
	try
	{
		PA_long32 pProcNum = selector;
		sLONG_PTR *pResult = (sLONG_PTR *)params->fResult;
		PackagePtr pParams = (PackagePtr)params->fParameters;

		CommandDispatcher(pProcNum, pResult, pParams); 
	}
	catch(...)
	{

	}
}

void CommandDispatcher (PA_long32 pProcNum, sLONG_PTR *pResult, PackagePtr pParams)
{
	switch(pProcNum)
	{
        case kInitPlugin :
        case kServerInitPlugin :
            OnStartup();
            break;
// --- Mac Color Picker

		case 1 :
			Mac_Select_RGB_color(pResult, pParams);
			break;

	}
}

// ------------------------------- Mac Color Picker -------------------------------

@interface Listener : NSObject
{
    CGFloat red;
    CGFloat green;
    CGFloat blue;
    CGFloat alpha;
    BOOL running;
    id target;
    SEL action;
}
- (id)init;
- (void)dealloc;
- (void)pickerWillClose:(id)sender;
- (void)colorDidChange:(NSNotification *)notification;
- (BOOL)isRunning;
- (void)getRed:(CGFloat *)red green:(CGFloat *)green blue:(CGFloat *)blue alpha:(CGFloat *)alpha;
@end

@implementation Listener
- (id)init
{
	if(!(self = [super init])) return self;
	
    NSColorPanel *sharedColorPanel = [NSColorPanel sharedColorPanel];
    
    [[NSNotificationCenter defaultCenter]
     addObserver:self
     selector:@selector(colorDidChange:)
     name:NSColorPanelColorDidChangeNotification
     object:sharedColorPanel];
    
     NSButton *closeButton = [[NSColorPanel sharedColorPanel] standardWindowButton:NSWindowCloseButton];
    
    target = closeButton.target;
    action = closeButton.action;
    
    [closeButton setTarget:self];
    [closeButton setAction:@selector(pickerWillClose:)];

    running = YES;
    
    [sharedColorPanel makeKeyAndOrderFront:nil];
    //force redraw, especially after changing the visibility status of the transparancy tool
    [[sharedColorPanel contentView]setNeedsDisplay:YES];
    
	return self;
}
- (void)dealloc
{
    [[NSNotificationCenter defaultCenter]removeObserver:self];
    NSColorPanel *sharedColorPanel = [NSColorPanel sharedColorPanel];
    NSButton *closeButton = [sharedColorPanel standardWindowButton:NSWindowCloseButton];
    //restore original, to avoid conflict with Select RGB color command
    [closeButton setTarget:target];
    [closeButton setAction:action];
    [super dealloc];
}
- (void)colorDidChange:(NSNotification *)notification
{
    NSColorPanel *sharedColorPanel = [NSColorPanel sharedColorPanel];
    NSColor *color = [sharedColorPanel color];
    [color getRed:&red green:&green blue:&blue alpha:&alpha];
}
- (void)pickerWillClose:(id)sender
{
    running = NO;
}
- (BOOL)isRunning
{
    return running;
}
- (void)getRed:(CGFloat *)r green:(CGFloat *)g blue:(CGFloat *)b alpha:(CGFloat *)a
{
    *r = red;
    *g = green;
    *b = blue;
    *a = alpha;
}
@end

namespace ColorPicker
{
    typedef struct
    {
        NSString *title;
        BOOL showsAlpha;
        CGFloat red;
        CGFloat green;
        CGFloat blue;
        CGFloat alpha;
        Listener *listener;
    }Params;
    void Close()
    {
        NSColorPanel *sharedColorPanel = [NSColorPanel sharedColorPanel];
        [sharedColorPanel close];
    }
    void Open(ColorPicker::Params *params){
        
        NSColorPanel *sharedColorPanel = [NSColorPanel sharedColorPanel];
        
        sharedColorPanel.showsAlpha = params->showsAlpha;
        sharedColorPanel.color = [NSColor colorWithDeviceRed:params->red green:params->green blue:params->blue alpha:params->alpha];
        sharedColorPanel.title = params->title;
        
         params->listener = [[Listener alloc]init];
    }
}

void Mac_Select_RGB_color(sLONG_PTR *pResult, PackagePtr pParams)
{
	C_LONGINT Param1;//rga(a)
	C_TEXT Param2;//windows title
	C_LONGINT Param3;//flags
	C_LONGINT returnValue;

	Param1.fromParamAtIndex(pParams, 1);
	Param2.fromParamAtIndex(pParams, 2);
	Param3.fromParamAtIndex(pParams, 3);

	NSUInteger color = Param1.getIntValue();
    NSString *title = Param2.copyUTF16String();
    NSUInteger flags = Param3.getIntValue();
    
    ColorPicker::Params params;
    
    params.showsAlpha   = (flags & 0x0001);
    
    params.alpha    = (CGFloat)((color & 0xFF000000) >> 24)  / 0xFF;
    params.red      = (CGFloat)((color & 0x00FF0000) >> 16)  / 0xFF;
	params.green    = (CGFloat)((color & 0x0000FF00) >> 8)   / 0xFF;
	params.blue     = (CGFloat)((color & 0x000000FF))        / 0xFF;
    
    params.title    = title;
    params.alpha    = params.showsAlpha ? params.alpha : 1.0f;
    
    PA_RunInMainProcess((PA_RunInMainProcessProcPtr)ColorPicker::Open, &params);
    
    while([params.listener isRunning] && !PA_IsProcessDying())
    { 
        PA_YieldAbsolute();
    }
    
    PA_RunInMainProcess((PA_RunInMainProcessProcPtr)ColorPicker::Close, NULL);

    CGFloat red, green, blue, alpha;
    
    [params.listener getRed:&red green:&green blue:&blue alpha:&alpha];
    [params.listener release];
    
    BOOL useStandardColorSpace = (flags & 0x0002);
    
    if(useStandardColorSpace){
        //convert to device color space
        CGFloat components[4] = {red, green, blue, alpha};
        NSColor *tempColor = [NSColor colorWithColorSpace:[NSColorSpace genericRGBColorSpace]
            components:components count:4];
        tempColor = [tempColor colorUsingColorSpace:[NSColorSpace sRGBColorSpace]];
        [tempColor getRed:&red green:&green blue:&blue alpha:&alpha];
    }
    
    color = params.showsAlpha ? ((NSUInteger)(alpha * 0xFF) << 24) : 0;
    
    color = color
        +((NSUInteger)(red      * 0xFF) << 16)
        +((NSUInteger)(green    * 0xFF) << 8)
        + (NSUInteger)(blue     * 0xFF);
    
    returnValue.setIntValue(color);
	returnValue.setReturn(pResult);
    
    [title release];
}

#pragma mark -

void OnStartup()
{
    //hide crayon on El Capitan (must do this before the application creates its first instance)
    if(NSAppKitVersionNumber > NSAppKitVersionNumber10_10_Max)
    {
        [NSColorPanel setPickerMask:0x007F];
    }else{
        [NSColorPanel setPickerMask:0xFFFF];
    }
}
