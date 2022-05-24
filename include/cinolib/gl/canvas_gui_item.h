#ifndef CINO_CANVAS_GUI_ITEM_H
#define CINO_CANVAS_GUI_ITEM_H

namespace cinolib
{

	class CanvasGuiItem
	{

	public:

		virtual  ~CanvasGuiItem() = default;
		virtual void draw() = 0;

	};

}

#endif // CINO_CANVAS_GUI_ITEM_H
