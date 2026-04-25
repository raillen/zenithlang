#![allow(dead_code)]

use iced::widget::button::Status as ButtonStatus;
use iced::widget::text_input::Status as TextInputStatus;
use iced::widget::{button, container, text, text_input};
use iced::{Background, Border, Color, Shadow, Theme, Vector};

const CORNER_RADIUS: f32 = 8.0;
const SMALL_CORNER_RADIUS: f32 = 6.0;
const TINY_CORNER_RADIUS: f32 = 4.0;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ButtonKind {
    Primary,
    Secondary,
    Success,
    Danger,
    System,
    Plain,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ContainerKind {
    App,
    Toolbar,
    Sidebar,
    Panel,
    Card,
    Viewport,
    Code,
}

pub struct Modern;

#[derive(Debug, Clone, Copy)]
struct Colors {
    blue: Color,
    green: Color,
    red: Color,
    orange: Color,
    text: Color,
    secondary_text: Color,
    tertiary_text: Color,
    background: Color,
    secondary_background: Color,
    tertiary_background: Color,
    card_background: Color,
    input_background: Color,
    input_border: Color,
    separator: Color,
    placeholder: Color,
    selection: Color,
}

impl Modern {
    pub fn light_theme() -> Theme {
        Theme::custom(
            "Modern Light",
            iced::theme::Palette {
                background: rgb(0xf2f2f7),
                text: Color::BLACK,
                primary: rgb(0x007aff),
                success: rgb(0x34c759),
                warning: rgb(0xff9500),
                danger: rgb(0xff3b30),
            },
        )
    }

    pub fn container(kind: ContainerKind) -> impl Fn(&Theme) -> container::Style {
        move |theme| container_style(theme, kind)
    }

    pub fn button(kind: ButtonKind) -> impl Fn(&Theme, ButtonStatus) -> button::Style {
        move |theme, status| button_style(theme, kind, status)
    }

    pub fn primary_button() -> impl Fn(&Theme, ButtonStatus) -> button::Style {
        Self::button(ButtonKind::Primary)
    }

    pub fn secondary_button() -> impl Fn(&Theme, ButtonStatus) -> button::Style {
        Self::button(ButtonKind::Secondary)
    }

    pub fn success_button() -> impl Fn(&Theme, ButtonStatus) -> button::Style {
        Self::button(ButtonKind::Success)
    }

    pub fn danger_button() -> impl Fn(&Theme, ButtonStatus) -> button::Style {
        Self::button(ButtonKind::Danger)
    }

    pub fn system_button() -> impl Fn(&Theme, ButtonStatus) -> button::Style {
        Self::button(ButtonKind::System)
    }

    pub fn plain_button() -> impl Fn(&Theme, ButtonStatus) -> button::Style {
        Self::button(ButtonKind::Plain)
    }

    pub fn text_input() -> impl Fn(&Theme, TextInputStatus) -> text_input::Style {
        text_input_style
    }

    pub fn primary_text() -> impl Fn(&Theme) -> text::Style {
        |theme| text::Style {
            color: Some(colors(theme).text),
        }
    }

    pub fn secondary_text() -> impl Fn(&Theme) -> text::Style {
        |theme| text::Style {
            color: Some(colors(theme).secondary_text),
        }
    }

    pub fn tertiary_text() -> impl Fn(&Theme) -> text::Style {
        |theme| text::Style {
            color: Some(colors(theme).tertiary_text),
        }
    }
}

fn container_style(theme: &Theme, kind: ContainerKind) -> container::Style {
    let c = colors(theme);

    match kind {
        ContainerKind::App => container::Style {
            text_color: Some(c.text),
            background: Some(Background::Color(c.background)),
            ..Default::default()
        },
        ContainerKind::Toolbar => container::Style {
            text_color: Some(c.text),
            background: Some(Background::Color(c.card_background)),
            border: Border {
                radius: 0.0.into(),
                width: 0.0,
                color: Color::TRANSPARENT,
            },
            shadow: Shadow {
                color: Color {
                    a: 0.08,
                    ..Color::BLACK
                },
                offset: Vector::new(0.0, 1.0),
                blur_radius: 4.0,
            },
            ..Default::default()
        },
        ContainerKind::Sidebar => container::Style {
            text_color: Some(c.text),
            background: Some(Background::Color(c.secondary_background)),
            border: Border {
                radius: 0.0.into(),
                width: 0.0,
                color: Color::TRANSPARENT,
            },
            ..Default::default()
        },
        ContainerKind::Panel => container::Style {
            text_color: Some(c.text),
            background: Some(Background::Color(c.secondary_background)),
            border: Border {
                radius: 0.0.into(),
                width: 0.0,
                color: Color::TRANSPARENT,
            },
            ..Default::default()
        },
        ContainerKind::Card => container::Style {
            text_color: Some(c.text),
            background: Some(Background::Color(c.card_background)),
            border: Border {
                radius: CORNER_RADIUS.into(),
                width: 1.0,
                color: c.separator.scale_alpha(0.7),
            },
            shadow: Shadow {
                color: Color {
                    a: 0.08,
                    ..Color::BLACK
                },
                offset: Vector::new(0.0, 2.0),
                blur_radius: 8.0,
            },
            ..Default::default()
        },
        ContainerKind::Viewport => container::Style {
            text_color: Some(Color::WHITE),
            background: Some(Background::Color(rgb(0x1c1f26))),
            border: Border {
                radius: TINY_CORNER_RADIUS.into(),
                width: 1.0,
                color: rgb(0x343943),
            },
            ..Default::default()
        },
        ContainerKind::Code => container::Style {
            text_color: Some(c.text),
            background: Some(Background::Color(c.card_background)),
            border: Border {
                radius: CORNER_RADIUS.into(),
                width: 1.0,
                color: c.separator,
            },
            ..Default::default()
        },
    }
}

fn text_input_style(theme: &Theme, status: TextInputStatus) -> text_input::Style {
    let c = colors(theme);
    let base = text_input::Style {
        background: Background::Color(c.input_background),
        border: Border {
            radius: SMALL_CORNER_RADIUS.into(),
            width: 1.0,
            color: c.input_border,
        },
        icon: c.text,
        placeholder: c.placeholder,
        value: c.text,
        selection: c.selection,
    };

    match status {
        TextInputStatus::Active => base,
        TextInputStatus::Hovered => text_input::Style {
            border: Border {
                color: c.placeholder,
                ..base.border
            },
            ..base
        },
        TextInputStatus::Focused { .. } => text_input::Style {
            border: Border {
                color: c.blue,
                width: 2.0,
                ..base.border
            },
            ..base
        },
        TextInputStatus::Disabled => text_input::Style {
            background: Background::Color(c.input_background.scale_alpha(0.65)),
            border: Border {
                color: c.input_border.scale_alpha(0.5),
                ..base.border
            },
            value: c.text.scale_alpha(0.5),
            ..base
        },
    }
}

fn button_style(theme: &Theme, kind: ButtonKind, status: ButtonStatus) -> button::Style {
    let c = colors(theme);
    let is_dark = is_dark_mode(theme);

    let filled = |background: Color, text_color: Color| button::Style {
        background: Some(Background::Color(background)),
        text_color,
        border: Border {
            radius: CORNER_RADIUS.into(),
            width: 0.0,
            color: Color::TRANSPARENT,
        },
        shadow: Shadow {
            color: Color {
                a: 0.10,
                ..Color::BLACK
            },
            offset: Vector::new(0.0, 1.0),
            blur_radius: 2.0,
        },
        ..Default::default()
    };

    let outlined = |border_color: Color, text_color: Color| button::Style {
        background: Some(Background::Color(Color::TRANSPARENT)),
        text_color,
        border: Border {
            radius: CORNER_RADIUS.into(),
            width: 1.0,
            color: border_color,
        },
        shadow: Shadow::default(),
        ..Default::default()
    };

    let transparent = |text_color: Color| button::Style {
        background: Some(Background::Color(Color::TRANSPARENT)),
        text_color,
        border: Border::default(),
        shadow: Shadow::default(),
        ..Default::default()
    };

    let base = match kind {
        ButtonKind::Primary => filled(c.blue, Color::WHITE),
        ButtonKind::Secondary => outlined(c.blue, c.blue),
        ButtonKind::Success => filled(c.green, Color::WHITE),
        ButtonKind::Danger => filled(c.red, Color::WHITE),
        ButtonKind::System => filled(c.tertiary_background, c.text),
        ButtonKind::Plain => transparent(c.text),
    };

    match status {
        ButtonStatus::Active => base,
        ButtonStatus::Hovered => hover_button(base, is_dark, matches!(kind, ButtonKind::Plain)),
        ButtonStatus::Pressed => pressed_button(base, is_dark, matches!(kind, ButtonKind::Plain)),
        ButtonStatus::Disabled => button::Style {
            background: base.background.map(|background| match background {
                Background::Color(color) => Background::Color(color.scale_alpha(0.5)),
                other => other,
            }),
            text_color: base.text_color.scale_alpha(0.5),
            border: Border {
                color: base.border.color.scale_alpha(0.5),
                ..base.border
            },
            shadow: Shadow::default(),
            ..Default::default()
        },
    }
}

fn hover_button(base: button::Style, is_dark: bool, text_only: bool) -> button::Style {
    if text_only {
        return button::Style {
            text_color: base.text_color.scale_alpha(0.8),
            ..base
        };
    }

    button::Style {
        background: base.background.map(|background| match background {
            Background::Color(color) => Background::Color(adjust_color(color, is_dark, 0.05)),
            other => other,
        }),
        ..base
    }
}

fn pressed_button(mut base: button::Style, is_dark: bool, text_only: bool) -> button::Style {
    base.shadow = Shadow::default();

    if text_only {
        base.text_color = base.text_color.scale_alpha(0.6);
        return base;
    }

    base.background = base.background.map(|background| match background {
        Background::Color(color) => Background::Color(adjust_color(color, is_dark, 0.10)),
        other => other,
    });
    base
}

fn adjust_color(color: Color, lighten: bool, amount: f32) -> Color {
    let adjust = |value: f32| {
        if lighten {
            (value + amount).min(1.0)
        } else {
            (value - amount).max(0.0)
        }
    };

    Color {
        r: adjust(color.r),
        g: adjust(color.g),
        b: adjust(color.b),
        a: color.a,
    }
}

fn colors(theme: &Theme) -> Colors {
    let is_dark = is_dark_mode(theme);

    Colors {
        blue: if is_dark {
            rgb(0x0a84ff)
        } else {
            rgb(0x007aff)
        },
        green: if is_dark {
            rgb(0x30d158)
        } else {
            rgb(0x34c759)
        },
        red: if is_dark {
            rgb(0xff453a)
        } else {
            rgb(0xff3b30)
        },
        orange: if is_dark {
            rgb(0xff9f0a)
        } else {
            rgb(0xff9500)
        },
        text: if is_dark { Color::WHITE } else { Color::BLACK },
        secondary_text: if is_dark {
            rgb(0xc7c7cc)
        } else {
            rgb(0x6d6d72)
        },
        tertiary_text: rgb(0x8e8e93),
        background: if is_dark {
            rgb(0x1c1c1e)
        } else {
            rgb(0xf2f2f7)
        },
        secondary_background: if is_dark {
            rgb(0x2c2c2e)
        } else {
            rgb(0xeaeaee)
        },
        tertiary_background: if is_dark {
            rgb(0x38383a)
        } else {
            rgb(0xe5e5ea)
        },
        card_background: if is_dark { rgb(0x2c2c2e) } else { Color::WHITE },
        input_background: if is_dark {
            rgb(0x2c2c2e)
        } else {
            rgb(0xf2f2f7)
        },
        input_border: if is_dark {
            rgb(0x48484a)
        } else {
            rgb(0xd1d1d6)
        },
        separator: if is_dark {
            rgb(0x545458)
        } else {
            rgb(0xc7c7cc)
        },
        placeholder: rgb(0x8e8e93),
        selection: if is_dark {
            rgb(0x0a84ff).scale_alpha(0.3)
        } else {
            rgb(0x007aff).scale_alpha(0.3)
        },
    }
}

fn is_dark_mode(theme: &Theme) -> bool {
    match theme {
        Theme::Dark => true,
        Theme::Custom(custom) => custom.to_string().contains("Dark"),
        _ => false,
    }
}

fn rgb(hex: u32) -> Color {
    let r = ((hex >> 16) & 0xff) as f32 / 255.0;
    let g = ((hex >> 8) & 0xff) as f32 / 255.0;
    let b = (hex & 0xff) as f32 / 255.0;
    Color::from_rgb(r, g, b)
}
