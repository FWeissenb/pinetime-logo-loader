initSidebarItems({"enum":[["Color","A datatype representing color."],["FixedGradient","Any fixed gradient."],["ImageFormat","The pixel format for bitmap images."],["InterpolationMode","A requested interpolation mode for drawing images."],["LineCap","Options for the cap of stroked lines."],["LineJoin","Options for angled joins in strokes."],["PaintBrush","A color or a gradient."],["ScaleMode","Mappings from the unit square into a non-square rectangle."]],"mod":[["color","A simple representation of color"],["conv","Conversions of fundamental numeric and geometric types."],["error","The common error type for piet operations."],["gradient","Gradient specifications."],["null_renderer","A render context that does nothing."],["render_context","The main render context trait."],["shapes","Options for drawing paths."],["text","Traits for fonts and text handling."]],"struct":[["Error","An error that can occur while rendering 2D graphics."],["FixedLinearGradient","Specification of a linear gradient."],["FixedRadialGradient","Specification of a radial gradient in image-space."],["GradientStop","Specification of a gradient stop."],["HitTestMetrics","Hit test metrics, returned as part of `hit_test_text_position` and `hit_test_point`."],["HitTestPoint","return values for `hit_test_point`."],["HitTestTextPosition","return values for `hit_test_text_position`."],["LinearGradient","A description of a linear gradient in the unit rect, which can be resolved to a fixed gradient."],["RadialGradient","A description of a radial gradient in the unit rect, which can be resolved to a fixed gradient."],["StrokeStyle","Options for drawing stroked lines."],["UnitPoint","A representation of a point relative to a unit rectangle."]],"trait":[["Font",""],["FontBuilder",""],["GradientStops","A flexible, ergonomic way to describe gradient stops."],["IntoBrush","A trait for various types that can be used as brushes. These include backend-independent types such `Color` and `LinearGradient`, as well as the types used to represent these on a specific backend."],["RenderContext","The main trait for rendering graphics."],["RoundFrom","This is our own implementation of a \"lossy From\" trait, representing a conversion that can have precision loss. It is essentially adapted from https://github.com/rust-lang/rfcs/pull/2484."],["RoundInto","The companion to `RoundFrom`. As with `From` and `Into`, a blanket implementation is provided; for the most part, implement `RoundFrom`."],["Text",""],["TextLayout","Text Layout"],["TextLayoutBuilder",""]]});