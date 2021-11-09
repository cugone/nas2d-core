// ==================================================================================
// = NAS2D
// = Copyright © 2008 - 2020 New Age Software
// ==================================================================================
// = NAS2D is distributed under the terms of the zlib license. You are free to copy,
// = modify and distribute the software under the terms of the zlib license.
// =
// = Acknowledgment of your use of NAS2D is appreciated but is not required.
// ==================================================================================

#pragma once

#include "Point.h"
#include "Vector.h"


namespace NAS2D
{
	template <typename BaseType>
	struct Rectangle
	{
		BaseType x = 0;
		BaseType y = 0;
		BaseType width = 0;
		BaseType height = 0;

		constexpr static Rectangle<BaseType> Create(Point<BaseType> startPoint, Vector<BaseType> size)
		{
			return {startPoint.x, startPoint.y, size.x, size.y};
		}

		constexpr static Rectangle<BaseType> Create(Point<BaseType> startPoint, Point<BaseType> endPoint)
		{
			return Create(startPoint, endPoint - startPoint);
		}

		constexpr bool operator==(const Rectangle& rect) const
		{
			return (x == rect.x) && (y == rect.y) && (width == rect.width) && (height == rect.height);
		}

		constexpr bool operator!=(const Rectangle& rect) const
		{
			return !(*this == rect);
		}

		constexpr Vector<BaseType> size() const
		{
			return {width, height};
		}

		constexpr Point<BaseType> startPoint() const
		{
			return {x, y};
		}

		constexpr Point<BaseType> endPoint() const
		{
			return Point{x, y} + Vector{width, height};
		}

		constexpr Point<BaseType> crossXPoint() const
		{
			return {x + width, y};
		}

		constexpr Point<BaseType> crossYPoint() const
		{
			return {x, y + height};
		}

		constexpr bool null() const
		{
			return (width == 0) || (height == 0);
		}

		void size(NAS2D::Vector<BaseType> newSize)
		{
			width = newSize.x;
			height = newSize.y;
		}

		void startPoint(NAS2D::Point<BaseType> newStartPoint)
		{
			x = newStartPoint.x;
			y = newStartPoint.y;
		}

		constexpr Rectangle translate(Vector<BaseType> offset) const
		{
			return Create(startPoint() + offset, size());
		}

		constexpr Rectangle operator+(Vector<BaseType> translation) const
		{
			return Create(startPoint() + translation, size());
		}

		constexpr Rectangle operator-(Vector<BaseType> translation) const
		{
			return Create(startPoint() - translation, size());
		}

		constexpr Rectangle& operator+=(Vector<BaseType> translation)
		{
			x += translation.x;
			y += translation.y;
			return *this;
		}

		constexpr Rectangle& operator-=(Vector<BaseType> translation)
		{
			x -= translation.x;
			y -= translation.y;
			return *this;
		}

		constexpr Rectangle inset(BaseType amount) const
		{
			return {x + amount, y + amount, width - 2 * amount, height - 2 * amount};
		}

		constexpr Rectangle inset(Vector<BaseType> amount) const
		{
			return {x + amount.x, y + amount.y, width - 2 * amount.x, height - 2 * amount.y};
		}

		constexpr Rectangle inset(Vector<BaseType> amountStart, Vector<BaseType> amountEnd) const
		{
			return {x + amountStart.x, y + amountStart.y, width - amountStart.x - amountEnd.x, height - amountStart.y - amountEnd.y};
		}

		constexpr Rectangle skewBy(const Vector<BaseType>& scaleFactor) const
		{
			return Create(startPoint().skewBy(scaleFactor), size().skewBy(scaleFactor));
		}

		constexpr Rectangle skewInverseBy(const Vector<BaseType>& scaleFactor) const
		{
			return Create(startPoint().skewInverseBy(scaleFactor), size().skewInverseBy(scaleFactor));
		}

		template <typename NewBaseType>
		constexpr operator Rectangle<NewBaseType>() const
		{
			return {
				static_cast<NewBaseType>(x),
				static_cast<NewBaseType>(y),
				static_cast<NewBaseType>(width),
				static_cast<NewBaseType>(height)
			};
		}

		template <typename NewBaseType>
		constexpr Rectangle<NewBaseType> to() const
		{
			return static_cast<Rectangle<NewBaseType>>(*this);
		}

		// Start point inclusive (x, y), endpoint exclusive (x + width, y + height)
		// Area in interval notation: [x .. x + width), [y .. y + height)
		constexpr bool contains(const Point<BaseType>& point) const
		{
			return startPoint() <= point && point < endPoint();
		}

		// Start point inclusive (x, y), endpoint exclusive (x + width, y + height)
		// Area in interval notation: [x .. x + width), [y .. y + height)
		constexpr bool overlaps(const Rectangle& rect) const
		{
			return startPoint() < rect.endPoint() && rect.startPoint() < endPoint();
		}

		constexpr Point<BaseType> center() const
		{
			return {x + (width / 2), y + (height / 2)};
		}
	};


	template <typename BaseType>
	Rectangle(BaseType, BaseType, BaseType, BaseType) -> Rectangle<BaseType>;
} // namespace NAS2D
