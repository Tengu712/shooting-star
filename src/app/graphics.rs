use super::*;

use crate::log::*;

pub enum Position {
    Center,
    CenterUI,
    TopLeftUI,
    BottomRightUI,
}

pub(super) fn configure(settings: &mut HashMap<String, String>) -> f32 {
    match settings.remove("window-scale") {
        Some(n) => n
            .parse::<f32>()
            .unwrap_or_else(|_| ss_error("window-scale is not a float number.")),
        None => 1.0,
    }
}

impl SStarApp {
    /// A method to do all pushed rendering tasks.
    /// It blocks the thread until the presentation completed.
    pub fn flush(&mut self) {
        self.vulkan_app
            .render(self.ub.as_ref(), &self.tasks)
            .unwrap();
        self.ub = None;
        self.tasks.clear();
    }

    /// A method to bind a bitmap texture.
    /// You must call it to draw a bitmap before you push a drawing task.
    pub fn bind_texture(&mut self, id: usize) {
        self.tasks.push(RenderTask::SetImageTexture(id));
    }

    /// A method to push a task for drawing a rectangle.
    /// The size of the rectangle is automaticaly scaled based on the scene size.
    pub fn draw(&mut self, mut pc: PushConstant, pos: Position) {
        match pos {
            Position::Center => (),
            Position::CenterUI => {
                pc.trs[0] -= self.base_width / 2.0;
                pc.trs[1] -= self.base_height / 2.0;
                pc.trs[2] = 0.0;
            }
            Position::TopLeftUI => {
                pc.trs[0] += pc.scl[0] / 2.0 - self.base_width / 2.0;
                pc.trs[1] += pc.scl[1] / 2.0 - self.base_height / 2.0;
                pc.trs[2] = 0.0;
            }
            Position::BottomRightUI => {
                pc.trs[0] += -pc.scl[0] / 2.0 - self.base_width / 2.0;
                pc.trs[1] += -pc.scl[1] / 2.0 - self.base_height / 2.0;
                pc.trs[2] = 0.0;
            }
        }
        pc.scl[0] *= self.scene_scale;
        pc.scl[1] *= self.scene_scale;
        pc.trs[0] *= self.scene_scale;
        pc.trs[1] *= self.scene_scale;
        pc.trs[2] *= self.scene_scale;
        self.tasks.push(RenderTask::Draw(pc));
    }

    /// A method to push a task for drawing a text.
    pub fn draw_text(&mut self, mut pc: PushConstant, pos: Position, id: usize, txt: &str) {
        let info = self.text_infos.get(&id).map(|n| n.get(txt));
        if let Some(Some(info)) = info {
            pc.scl[0] *= info.width;
            pc.scl[1] *= info.height;
            pc.uv = info.uv;
            self.draw(pc, pos);
        } else {
            ss_warning(&format!(
                "the texture {id} not loaded or the text '{txt}' not in it."
            ));
        }
    }

    /// A method to push a task for drawing a text directly.
    /// The texture `id` should be a texture atlas that collects charactors.
    pub fn draw_chars(&mut self, pc: PushConstant, pos: Position, id: usize, txt: &str) {
        let mut ox = 0.0;
        let mut pcs = Vec::with_capacity(txt.chars().count());
        for c in txt.chars() {
            let s = c.to_string();
            let info = self.text_infos.get(&id).map(|n| n.get(&s));
            if let Some(Some(info)) = info {
                let mut pc = pc.clone();
                pc.scl[0] *= info.width;
                pc.scl[1] *= info.height;
                pc.trs[0] += ox;
                pc.uv = info.uv;
                pcs.push(pc);
                ox += info.width;
            } else {
                ss_warning(&format!(
                    "the texture {id} not loaded or the text '{txt}' not in it."
                ));
            }
        }
        for mut pc in pcs {
            match pos {
                Position::Center | Position::CenterUI => {
                    pc.trs[0] -= ox / 2.0;
                    pc.trs[1] -= pc.scl[1] / 2.0;
                }
                Position::TopLeftUI => (),
                Position::BottomRightUI => {
                    pc.trs[0] -= ox;
                    pc.trs[1] -= pc.scl[1];
                }
            }
            self.draw(pc, Position::TopLeftUI);
        }
    }
}
