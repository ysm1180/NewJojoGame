require "toolbar.lua"

ToolbarManager = Object:Instance
{
    CreateToolbar = function(self, options)
        local newControl = nil
        if options ~= nil then
            newControl = Toolbar:New()

            newControl:Create(options.Parent, options.Image.Width, options.Image.Height)

            for i = 1, #options.Buttons do
                newControl:AddButton(options.Buttons[i])
            end

            if options.Show then
                newControl:Show()
            end
        end

        return newControl
    end,

    CreateToolbarButton = function(self, options)
        local newControlButton = ToolbarButton:New()

        if options.Enabled == false then
            newControlButton:Disable()
        end

        if options.Text ~= nil then
            newControlButton:SetText(options.Text.Content)
            newControlButton:SetTooltipText(options.Text.Tooltip)
        end

        if options.Event then
            newControlButton:SetClickEvent(options.Event.Click)
        end
        
        newControlButton:SetImage(options.Image)

        newControlButton:Create()

        if options.Show then
            newControlButton:Show()
        end

        return newControlButton
    end
}