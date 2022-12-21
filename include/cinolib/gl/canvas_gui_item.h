#ifndef CINO_CANVAS_GUI_ITEM_H
#define CINO_CANVAS_GUI_ITEM_H

namespace cinolib
{

	class GLcanvas;

	class CanvasGuiItem
	{

	public:

		virtual ~CanvasGuiItem() = default;
		virtual void draw(const GLcanvas& _canvas) = 0;

	};

}

#endif // CINO_CANVAS_GUI_ITEM_H
